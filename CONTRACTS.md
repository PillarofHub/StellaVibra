# MVP 계약 (형식 1)

대상은 MSFS 2024, Windows 10/11 x64, C++20/MSVC, Qt Widgets 6.8.3 동적 링크, Lua 5.4.8 정적 링크다. Stella 소스는 MIT다. Qt 및 Microsoft 구성요소의 라이선스를 MIT로 재지정하지 않는다.

## 파일

프로젝트 `.sv`: `{"version":1,"id":"UUID","name":"Flight","game":"MSFS2024","effects":[{"id":"UUID","script":"effects/ground_roll.lua","enabled":true,"parameters":{"Strength":20},"outputs":{"Left Gear":["Throttle:1"],"Right Gear":["Throttle:2"]}}]}`.

기체 `aircrafts/UUID.sv`: `{"version":1,"aircraft_key":"정확한 TITLE","effects":{"효과 UUID":{"enabled":false,"parameters":{"Strength":10},"outputs":{"Left Gear":[]}}}}`.
누락은 상속, false와 []는 명시값. 출력은 이름별 전체 교체. Reset to Global은 해당 키 삭제. 일치하는 기체 파일이 두 개면 실행 거부. 기체 키는 같은 샘플의 TITLE 문자열 전체다. TITLE을 정규화·부분 매칭하지 않는다.

로컬 파일은 Qt AppLocalDataLocation의 `local.json` (`%LOCALAPPDATA%/StellaVibra/StellaVibra/local.json`): `{"version":1,"simconnect_dll":"절대 경로","motors":{"장치키/Left":{"slots":["Throttle:1"],"manual":false}},"geometry":"base64","layout":"base64"}`. 물리 식별은 VID/PID+일련번호. 일련번호가 없거나 중복이면 경로 기반 임시 키로 표시하고 자동 배정하지 않으며 매 앱 세션 수동 확인이 필요하다. 의도적 해제는 manual=true와 빈 slots로 보존한다. 예약된 매핑도 자동 배정 시 점유로 센다. 슬롯은 Yoke/Stick/Throttle/Pedal/Wheel 각각 :1–:4다.

JSON 상위 버전·잘못된 자료형·경로 이탈·중복 ID는 거부한다. 프로젝트 기준 상대 Lua 경로만 허용한다. QSaveFile로 파일별 원자 교체하며 프로젝트 전체 다중 파일 트랜잭션은 아니다. 미해결 파라미터/포트 키는 보존하고 UI에 표시한다.

## Lua

파일은 `local effect = {}; function effect:init(api) ... end; function effect:update(context) ... end; return effect` 형식이다. self는 해당 인스턴스의 테이블이다. init에서만 `api:parameter{name=string,default=number,min=number,max=number,step=positive number}` 또는 boolean default를 등록한다. 숫자는 유한하고 범위 안이어야 한다. 이름 중복은 오류다. `api:output(name)`은 이름 문자열 포트를 등록한다.

update의 `context.parameters`는 default→global→현재 기체 합성값이다. `context:log(text)`, `context:vibrate(name,constant(strength),duration_ms)`, `context:vibrate(name,pwm{strength=...,on_ms=...,period_ms=...},duration_ms)`, `context:vibrate(name,off())`를 제공한다. 세기는 정수 0–255, 기간은 0초과 60000ms 이하, PWM은 0≤ON≤period≤60000, period≥20ms다. OFF에는 기간이 없다. 정상 콜백이 끝난 뒤 명령 전체를 적용하며 오류 시 해당 효과 출력을 해제한다.

Lua 라이브러리는 base/math/string/table/utf8만 허용하고 파일·프로세스·네트워크·package/debug/coroutine/load/pcall/xpcall/setmetatable/getmetatable을 제거한다. Lua는 C++로 컴파일해 오류 시 호스트 객체도 예외 해제한다. 인스턴스 Lua 할당 메모리 8MiB, 콜백 200000명령 또는 20ms(1000명령마다 검사하며 네이티브 라이브러리 호출의 하드 실시간 선점은 아님), 소스 256KiB, 등록 각각 64개, 콜백 출력 128개와 로그 8개로 제한한다. top-level과 init도 보호 호출 안에서 실행한다.

## 입력과 실행

UI, Lua/SimConnect 실행 스레드, HID 스레드를 분리한다. SimConnect 호출은 실행 스레드에 한정한다. 입력 프레임은 최대 64개씩 직렬 처리한다. backlog 초과 시 연결을 끊고 출력을 정리한다. 새 입력은 수신 직후 update한다. 100ms마다 최신 스냅샷을 재전달하되 sample_id와 received_ms를 변경하지 않는다. now_ms/elapsed_ms는 호스트 steady_clock ms다. telemetry_age_ms는 실제 수신 기준, telemetry_valid는 수신 후 500ms 이내이고 sim 시간이 최근 500ms 안에 전진했으며 pause가 아닐 때만 true다. 기체 변경·시간 역행·pause·오류·Stop은 출력과 효과 상태를 초기화한다. 가속 시 파형과 Touchdown 지속 시간은 벽시계 기준이다.

telemetry: `sample_id, received_ms, sim_time_s, aircraft_key, ground_speed_knots, on_ground, gears={ {contact=bool,animation_percent=number}, ... }`, gears 순서는 Center/Left/Right다. MSFS 전용 값은 `msfs={paused=bool}`. 누락값은 nil이며 첫 샘플 전 유효하지 않다. 중복 시각도 실제 패킷이면 is_new_sample=true, `sim_time_advanced=false`; 재전달은 둘 다 false다. 완전 중복 패킷은 폐기해 수신 시각을 연장하지 않는다.

## 출력

HID 20ms. 기간은 update now+duration으로 교체한다. PWM 시간은 floor(ms/20+0.5)*20, 유효값으로 순위 비교. PWM→PWM은 만료되지 않은 기존 위상 유지, 다른 모드→PWM은 ON부터 시작. 요청 선택은 Constant 우선, 세기 내림차순, PWM 주기 오름차순·ON 내림차순. 완전 동률은 현재 승자 유지, 최초는 효과 UUID/포트 이름 사전순. 선택 후 파형을 계산하며 OFF 구간을 다른 후보로 채우지 않는다.

실행기 heartbeat 250ms 초과, 입력 500ms 만료, Stop 시 HID 스레드가 출력 해제. 정상 heartbeat는 기존 예약을 연장하지 않는다. generation 변경 후 낡은 명령은 버린다. 테스트는 모터 하나, 명시적 Start, 최대 5000ms이며 그 모터 게임 출력을 제외한다. 창을 열기만 하면 출력하지 않는다. 전역 Stop은 테스트도 종료한다.

## 효과

실제 예제는 `effects/ground_roll.lua`, `effects/touchdown.lua`. Ground Roll은 속도/접촉 게이트와 짧은 재요청을 Lua에서 수행한다. Touchdown은 바퀴마다 첫 airborne 관측 후 재무장한다. 접촉 상승에서 시작하여 첫 평탄/하강한 새로운 sim-time 샘플에서 첫 상승 시간×배수를 min/max로 제한해 최초 접촉 벽시계 기준 기한을 확정한다. 이후 피크는 무시한다. 중복 sim-time 접촉 변화는 수용하지만 상승 종료 근거로 사용하지 않는다. 압축 애니메이션은 힘이 아니며 미지원/정적인 기체는 최소 펄스를 사용한다. 확정 전에는 min과 관측된 상승 시간에 맞춰 스크립트가 재요청하며 호스트는 연장하지 않는다.

## 연동 근거

- [MSFS 2024 SimConnect](https://docs.flightsimulator.com/msfs2024/retail/programming-apis/simconnect/simconnect-sdk/): 동적 로드로 Open/Close/AddToDataDefinition/RequestDataOnSimObject/GetNextDispatch/SubscribeToSystemEvent 사용. SDK 미설치로 공식 공개 ABI 최소 선언을 사용한다. DLL은 Settings 경로를 우선하고, 미설정 시 EXE 옆 → MSFS2024_SDK → 기본 SDK 폴더를 탐색한다. 공식 SDK 클라이언트를 로컬 Release에만 복사한다. 공개 dist에는 Microsoft DLL을 포함하지 않는다(SIMCONNECT-LICENSING.md).
- [수신 ID](https://docs.flightsimulator.com/msfs2024/retail/programming-apis/simconnect/api-reference/structures-and-enumerations/simconnect_recv_id/): EXCEPTION=1. Aurora의 5 선언을 그대로 복사하지 않는다. [자료형 정의](https://docs.flightsimulator.com/msfs2024/retail/programming-apis/simconnect/api-reference/structures-and-enumerations/simconnect_datatype/)에 따라 STRING256=9이며 Aurora의 12 선언도 사용하지 않는다.
- [기어 변수](https://docs.flightsimulator.com/msfs2024/retail/programming-apis/simvars/aircraft-simvars/aircraft-brake-and-landing-gear-variables/): GEAR IS ON GROUND:0/1/2 Bool, GEAR ANIMATION POSITION:0/1/2 Percent. ABSOLUTE TIME Seconds, GROUND VELOCITY Knots, SIM ON GROUND Bool, TITLE STRING256.
- HID: Aurora `src/hid/protocol.rs` 및 HANDOFF. VID 4098/PID B970, Left=(04,0E), Right=(08,10). `[reportID,ident,B9,0,0,03,49,address,strength,0,0,0,0,0]`, reportID=02를 descriptor에서 확인하고 최소 길이14를 요구한다. 다른 장치는 출력하지 않는다. 비동기 WriteFile 완료/실패를 확인하고 제한 시간 초과 시 취소한다.

저장과 Apply는 분리한다. 편집 초안은 저장해도 실행되지 않는다. Apply는 모든 대상 효과를 새 Lua 상태에서 검증한 뒤 교체하며 실패하면 기존 버전 유지. Disable과 Stop은 즉시 반영한다.
