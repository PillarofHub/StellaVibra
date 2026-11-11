# MVP 계약 (형식 1)

대상은 MSFS 2024, Windows 10/11 x64, C++20/MSVC, Qt Widgets 6.8.3 동적 링크, Lua 5.4.8 정적 링크다. Stella 소스는 MIT다. Qt 및 Microsoft 구성요소의 라이선스를 MIT로 재지정하지 않는다.

## 파일

프로젝트 `.sv`: `{"version":1,"id":"UUID","name":"Flight","game":"MSFS2024","effects":[{"id":"UUID","script":"effects/ground/ground_roll.lua","enabled":true,"parameters":{"Strength":20},"outputs":{"Left Gear":["Throttle:1"],"Right Gear":["Throttle:2"]}}]}`.

기체 `aircrafts/UUID.sv`: `{"version":1,"aircraft_key":"정확한 TITLE","effects":{"효과 UUID":{"enabled":false,"parameters":{"Strength":10},"outputs":{"Left Gear":[]}}}}`.
누락은 상속, false와 []는 명시값. 출력은 이름별 전체 교체. Reset to Global은 해당 키 삭제. 일치하는 기체 파일이 두 개면 실행 거부. 기체 키는 같은 샘플의 TITLE 문자열 전체다. TITLE을 정규화·부분 매칭하지 않는다.

로컬 파일은 Qt AppLocalDataLocation의 `local.json` (`%LOCALAPPDATA%/StellaVibra/StellaVibra/local.json`): `{"version":1,"last_project":"절대 경로","recent_projects":["절대 경로"],"motors":{"장치키/Left":{"slots":["Throttle:1"],"manual":false}},"geometry":"base64","layout":"base64"}`. 물리 식별은 VID/PID+일련번호. 일련번호가 없거나 중복이면 경로 기반 임시 키로 표시하고 자동 배정하지 않으며 매 앱 세션 수동 확인이 필요하다. 의도적 해제는 manual=true와 빈 slots로 보존한다. 예약된 매핑도 자동 배정 시 점유로 센다. 슬롯은 Yoke/Stick/Throttle/Pedal/Wheel 각각 :1–:4다.

JSON 상위 버전·잘못된 자료형·경로 이탈·중복 ID는 거부한다. 프로젝트 기준 상대 Lua 경로만 허용한다. QSaveFile로 파일별 원자 교체하며 프로젝트 전체 다중 파일 트랜잭션은 아니다. 미해결 파라미터/포트 키는 보존하고 UI에 표시한다.

## Lua

파일은 `local effect = {}; function effect:init(context) ... end; function effect:update(context, telemetry, parameters) ... end; return effect` 형식이다. self는 해당 인스턴스의 테이블이다. init에서만 `context:parameter{name=string,type="boolean"|"int"|"float"|"haptic_power",default=...}`를 등록한다. int/float은 min/max/step이 필수이며 값은 유한하고 범위 안이어야 한다. int는 기본값·범위·step·저장값 모두 정수다. type은 필수다. 이름 중복은 오류다. `context:output(name)`은 이름 문자열 포트를 등록한다.

update의 `parameters`는 default→global→현재 기체 합성값이다. `context:log(text)`, `context:vibrate(name,constant(strength),duration_ms)`, `context:vibrate(name,pwm{strength=...,on_ms=...,period_ms=...},duration_ms)`, `context:vibrate(name,off())`를 제공한다. 세기는 정수 0–255, 기간은 0초과 60000ms 이하, PWM은 0≤ON≤period≤60000, period≥20ms다. OFF에는 기간이 없다. 정상 콜백이 끝난 뒤 명령 전체를 적용하며 오류 시 해당 효과 출력을 해제한다.

Lua 라이브러리는 base/math/string/table/utf8만 허용하고 파일·프로세스·네트워크·package/debug/coroutine/load/pcall/xpcall/setmetatable/getmetatable을 제거한다. Lua는 C++로 컴파일해 오류 시 호스트 객체도 예외 해제한다. 인스턴스 Lua 할당 메모리 8MiB, 콜백 200000명령 또는 20ms(1000명령마다 검사하며 네이티브 라이브러리 호출의 하드 실시간 선점은 아님), 소스 256KiB, 등록 각각 64개, 콜백 출력 128개와 로그 8개로 제한한다. top-level과 init도 보호 호출 안에서 실행한다.

## 입력과 실행

UI, Lua/SimConnect 실행 스레드, HID 스레드를 분리한다. SimConnect 호출은 실행 스레드에 한정한다. 입력 프레임은 최대 64개씩 직렬 처리한다. backlog 초과 시 연결을 끊고 출력을 정리한다. 새 입력은 수신 직후 update한다. 100ms마다 최신 스냅샷을 재전달하되 sample_id와 received_ms를 변경하지 않는다. context:now()/elapsed()는 호스트 steady_clock ms다. 입력 age는 context:now()-telemetry.received_ms이며 context:input_valid()는 수신 후 500ms 이내이고 sim 시간이 최근 500ms 안에 전진했으며 pause가 아닐 때만 true다. 기체 변경·시간 역행·pause·오류·Stop은 출력과 효과 상태를 초기화한다. 가속 시 파형과 Touchdown 지속 시간은 벽시계 기준이다.

telemetry: `sample_id, received_ms, sim_time_s, aircraft_key, ground_speed_knots, on_ground, gears={ {contact=bool,animation_percent=number}, ... }`, gears 순서는 Center/Left/Right다. MSFS 전용 값은 `msfs={paused=bool}`. 누락값은 nil이며 첫 샘플 전 유효하지 않다. 중복 시각도 실제 패킷이면 context:sample_is_new()=true, `sim_time_advanced=false`; 재전달은 둘 다 false다. 완전 중복 패킷은 폐기해 수신 시각을 연장하지 않는다.

## 출력

HID 기본 20ms, PC 로컬 tick_ms=5..100ms. 기간은 update now+duration으로 교체한다. PWM 시간은 floor(ms/tick_ms+0.5)*tick_ms, 유효값으로 순위 비교. PWM→PWM은 만료되지 않은 기존 위상 유지, 다른 모드→PWM은 ON부터 시작. 요청 선택은 Constant 우선, 세기 내림차순, PWM 주기 오름차순·ON 내림차순. 완전 동률은 현재 승자 유지, 최초는 효과 UUID/포트 이름 사전순. 선택 후 파형을 계산하며 OFF 구간을 다른 후보로 채우지 않는다.

실행기 heartbeat 250ms 초과, 입력 500ms 만료, Stop 시 HID 스레드가 출력 해제. 정상 heartbeat는 기존 예약을 연장하지 않는다. generation 변경 후 낡은 명령은 버린다. 테스트는 모터 하나, 명시적 Start, 최대 5000ms이며 그 모터 게임 출력을 제외한다. 창을 열기만 하면 출력하지 않는다. 전역 Stop은 테스트도 종료한다.

## 효과

실제 예제는 `effects/ground/ground_roll.lua`, `effects/ground/touchdown.lua`. Ground Roll은 속도/접촉 게이트와 짧은 재요청을 Lua에서 수행한다. Touchdown은 바퀴마다 첫 airborne 관측 후 재무장한다. 접촉 상승에서 시작하여 첫 평탄/하강한 새로운 sim-time 샘플에서 첫 상승 시간×배수를 min/max로 제한해 최초 접촉 벽시계 기준 기한을 확정한다. 이후 피크는 무시한다. 중복 sim-time 접촉 변화는 수용하지만 상승 종료 근거로 사용하지 않는다. 압축 애니메이션은 힘이 아니며 미지원/정적인 기체는 최소 펄스를 사용한다. 확정 전에는 min과 관측된 상승 시간에 맞춰 스크립트가 재요청하며 호스트는 연장하지 않는다.

## 연동 근거

- [MSFS 2024 SimConnect](https://docs.flightsimulator.com/msfs2024/retail/programming-apis/simconnect/simconnect-sdk/): 동적 로드로 Open/Close/AddToDataDefinition/RequestDataOnSimObject/GetNextDispatch/SubscribeToSystemEvent 사용. SDK 미설치로 공식 공개 ABI 최소 선언을 사용한다. DLL은 EXE 옆 SimConnect.dll만 로드한다. 공식 SDK 클라이언트를 로컬 Release/dist에 복사한다. 공개 재배포 권리 확인은 SIMCONNECT-LICENSING.md를 따른다.
- [수신 ID](https://docs.flightsimulator.com/msfs2024/retail/programming-apis/simconnect/api-reference/structures-and-enumerations/simconnect_recv_id/): EXCEPTION=1. Aurora의 5 선언을 그대로 복사하지 않는다. [자료형 정의](https://docs.flightsimulator.com/msfs2024/retail/programming-apis/simconnect/api-reference/structures-and-enumerations/simconnect_datatype/)에 따라 STRING256=9이며 Aurora의 12 선언도 사용하지 않는다.
- [기어 변수](https://docs.flightsimulator.com/msfs2024/retail/programming-apis/simvars/aircraft-simvars/aircraft-brake-and-landing-gear-variables/): GEAR IS ON GROUND:0/1/2 Bool, GEAR ANIMATION POSITION:0/1/2 Percent. ABSOLUTE TIME Seconds, GROUND VELOCITY Knots, SIM ON GROUND Bool, TITLE STRING256.
- HID: Aurora `src/hid/protocol.rs` 및 HANDOFF. VID 4098/PID B970은 Left=(04,0E), Right=(08,10). `[reportID,ident,B9,0,0,03,49,address,strength,0,0,0,0,0]`, B970은 reportID=02를 descriptor에서 확인한다. 모든 지원 장치는 길이≥14, UsagePage=1/Usage=4를 요구한다. 다른 모델의 목록·형식은 아래 확장 계약을 따른다. 비동기 WriteFile 완료/실패를 확인하고 제한 시간 초과 시 취소한다.

## 편집·파일 추적·자동 실행

File에 New/Open/Open Recent/Save/Save All/Close File/Close Project를 둔다. 시작 시 last_project만 복원하며 파일이 없으면 프로젝트 없는 상태다. Save는 현재 스크립트와 해당 효과의 글로벌/모든 기체 설정만 저장한다. 다른 효과의 초안은 저장하지 않는다. Save All은 전체 저장한다. 스크립트는 기체 공통이며 Parameters 상단의 편집 기체 선택은 코드 저장에 영향을 주지 않는다.

파라미터는 즉시 합성·적용하며 Lua self를 유지한다. 코드 저장 또는 외부 파일 내용 변경은 해당 효과만 검증·교체한다. 각 효과의 모든 프로필을 검증하고 실패하면 그 효과의 이전 유효 버전을 유지한다. 다른 효과의 갱신을 막지 않는다. 코드 초안은 실행하지 않는다. 출력 경로/활성화 변경은 해당 요청을 정리한다. PWM 설정 변경은 활성 PWM 위상을 보존한다.

effects 하위 Lua를 1초마다 추적한다. 누락 경로와 새 경로가 같은 파일명으로 각각 유일하게 대응되면 ID·설정을 이전한다. 대응되지 않는 누락 효과와 모든 기체 Override는 삭제한다. 새 경로는 새 ID, enabled=true, 빈 parameters/outputs(스크립트 기본값·미배정 출력)로 등록한다. 구조 변경만 자동 저장하고 다른 효과의 파라미터 초안은 저장하지 않는다. 실제 파일 목록은 QFileSystemModel로 갱신한다. 프로젝트 JSON의 외부 동시 편집 병합은 제공하지 않는다. 삭제 파일의 미저장 코드 버퍼는 보존하지만 효과 설정은 삭제한다. 사용자가 버퍼를 Save하여 파일을 재생성하면 새 효과/기본값으로 등록된다.

MSFS는 미연결 시 2초마다 재시도한다. 연결 중에는 [RequestSystemState](https://docs.flightsimulator.com/html/Programming_Tools/SimConnect/API_Reference/General/SimConnect_RequestSystemState.htm)의 Sim 요청을 2초 간격으로 보내고 응답이 5초간 없으면 재연결한다. 이 응답은 텔레메트리 수신/유효 시각을 바꾸지 않는다. HID는 1초마다 재검색한다. 기존 HID 핸들은 유지한다. 실행은 기본 활성 상태이며 유효한 프로젝트·입력·매핑·효과 요청이 있을 때만 출력한다. Stop All은 요청/테스트를 해제하고 Resume All 전까지 게임 출력을 차단한다. 프로젝트 전환은 출력을 정리하되 전체 중지 선택은 유지한다. 테스트 창 열기만으로 새 테스트 출력을 만들지 않는다.

## haptic_power 파라미터

```lua
context:parameter {
    name="Power", type="haptic_power",
    default={strength=20, pwm=false, on_ms=20, period_ms=80},
    min=0, max=255
}
```

default 테이블은 필수. 테이블 내 생략값은 strength=0, pwm=false, on_ms=20, period_ms=80이다. min/max는 선택이며 0/255가 기본, 정수 0..255 안으로 제한한다. strength는 지정 범위 내 정수, PWM은 boolean, ON/period는 정수 ms이며 0≤ON≤period, 20≤period≤60000이다. UI는 강도 슬라이더+직접 숫자 입력, PWM 체크, 체크 시 ON/period 입력을 표시한다. 실행 시간은 설정한 HID tick 반올림 규칙을 따른다. 일반 숫자도 슬라이더+직접 입력이며 step은 슬라이더/증감 간격이다(직접 입력값을 step 배수로 강제하지 않는다). 슬라이더 분할은 최대 1,000,000이다.

저장 및 context.parameters의 Power는 `{strength:20,pwm:false,on_ms:20,period_ms:80}` 객체다. 복합 파라미터 하나가 상속·Override의 단위이며 내부 필드별 부분 상속은 하지 않는다. Reset All to Global은 선택 효과의 현재 기체 Override 전체를 지운다. 예제는 이 객체에서 constant/pwm 사양을 만들며 파형 선택·게임 판단은 Lua에 남긴다.

## 도킹 UI·HID 확장 (2026-11-10)

View 메뉴의 Project Explorer / Effect Control / Motor Monitor / Debug Output은 각 QDockWidget 표시 상태를 제어한다. Effect Control에 Parameters/Script Editor를 함께 둔다. 상태는 local.json layout에 보존하며 layout_version=2는 새 기본 크기 배치의 적용 여부다. 숨김은 실행 중지를 의미하지 않는다. Monitor는 연결 모터만 Motor/Slots/HID 세 열로 표시한다. HID는 마지막 성공 전송 강도이며 20ms UI 갱신은 하드 실시간/모든 펄스 관측 보장이 아니다. 연결 해제 후 PC 매핑 예약은 유지한다.

Aurora protocol.rs의 11개 PID를 허용한다(모두 VID 4098): BC27–BC2C, B920, B930, B970, BEA8, BD64. 스틱 category=Stick, 스로틀 category=Throttle. 기존 B970 모터 키/좌우 매핑은 유지한다. 스틱은 /Stick, Orion 스로틀은 /Combined 한 출력이며 URSA 스로틀은 /Left와 /Right다.

공통 프레임은 `[report,identifier,command,0,0,03,49,address,strength,0,0,0,0,0]`, descriptor 길이까지 0 패딩한다. BC27–BC2C는 identifier=07–0C, command=BF, address=00. B920/B930은 identifier=10, command=B9, address=Left 0E/Right 10. B970은 기존 (04,0E)/(08,10), command=B9. BEA8은 identifier=01, command=00, address=00. BD64는 identifier=01/address=00에서 command=BF와 CF 두 보고서에 같은 선택 강도를 보낸다. 별도 Left/Right 제어로 가장하지 않는다.

B970 이외에는 Aurora의 최소 비영(없으면 0) output report ID 선택을 따르되 value/button caps에서 실제 확인된 ID만 사용한다. descriptor 조회 실패 시 14/02를 추정하는 Aurora fallback은 사용하지 않는다. 알려지지 않은 PID, 호환되지 않는 descriptor, 쓰기 접근 실패를 로그로 알려주며 같은 오류를 매 스캔마다 반복하지 않는다. 부분 전송 실패도 출력을 시도한 장치로 기억해 재연결 후 필요하면 0 전송을 재시도한다. 이는 실제 물리 정지 확인을 뜻하지 않는다.

## 단일 Explorer·파일 탭·JSON 편집 (2026-11-10 후속)

QFileSystemModel 기반 트리에서 등록된 Lua만 CheckStateRole을 가진다. 체크박스는 현재 편집 프로필의 enabled를 수정하고 바로 반영한다. 디렉터리 체크/일괄 활성화는 제공하지 않는다. 상태 갱신은 dataChanged로 전달하고 모델/트리를 초기화하지 않아 펼침 상태를 유지한다.

Effect Control 최상단 QTabBar와 편집 문서 QStackedWidget은 동일 순서·선택을 유지한다. 탭 아래에 공통 Parameters/Script Editor 분할 영역을 둔다. 각 파일 탭은 별도 Editor/QTextDocument를 갖는다. 상대 경로로 탭을 식별하고 이동 감지 시 해당 탭의 경로를 바꾸며 커서·undo를 유지한다. raw `.sv`에서는 Parameters를 숨기고 자동 생성 파일 안내와 JSON 강조를 표시한다. 루트 프로젝트 파일과 로드된 aircraft 파일의 저장만 허용한다.

raw Save는 열린 시점(또는 마지막 자동 갱신)의 디스크 문자열을 기준으로 충돌을 검사한다. JSON 초안 전체를 4MiB 이내 객체로 파싱하고, 실제 파일을 쓰기 전에 해당 객체들을 대입한 Project::load로 프로젝트/모든 기체 스키마를 검증한다. 성공 후 해당 파일만 QSaveFile로 쓰고 메모리 Project와 실행 설정을 다시 반영한다. Save All의 복수 JSON도 모두 검증한 뒤 저장하되 다중 파일 저장 전체가 트랜잭션인 것은 아니다. 문법/스키마 오류는 초안과 원본을 유지한다. Lua의 범위/실행 검증 실패는 기존 효과별 오류 격리 규칙을 따른다.

파라미터 초안이 있으면 raw 문서는 읽기 전용이고, raw 초안이 있으면 파라미터/체크 변경 및 effects 구조 자동 반영은 보류한다. 마지막 유효 실행은 유지한다. raw 문서를 수정하지 않았으면 자동 생성 저장 시 열린 문서도 갱신한다. 코드 초안과 JSON 초안은 동시에 열 수 있다. JSON 저장 시 Lua 코드 초안은 저장하지 않는다.

Monitor 셀 동작은 Slots=매핑, HID=테스트(우클릭/더블클릭). 목록은 logicalSlots의 최소 매핑 인덱스 → 대소문자 무시 장비 이름 → 장치 ID순이며 미배정은 마지막이다. 선택은 행 번호가 아닌 ID로 복원하므로 재정렬 중 다른 모터로 바뀌지 않는다. 테스트는 여전히 명시적 Start가 필요하다.

## 최소 설정 표현·수정 상태 (2026-11-10 후속)

프로젝트 enabled 누락은 true다. 글로벌 파라미터 누락은 Lua default, 출력 이름 누락은 빈 목적지 목록이다. Specific 누락은 global/default 합성값을 상속한다. Global에서 default와 동일한 항목, Specific에서 현재 global 합성과 동일한 항목은 생략한다. 출력 배열과 haptic_power 객체는 전체 값 단위로 비교한다. enabled=false 또는 outputs=[]도 상위와 다르면 명시값으로 보존한다. 빈 parameters/outputs 그룹, 빈 효과 Override는 제거한다. 알 수 없는 파라미터/출력은 기본값을 추정해 제거하지 않는다.

UI 설정 조절/메타데이터 확인 시 최소 표현으로 정리하고 일반 Save/Save All에서 파일에 반영한다. 스크립트 오류로 기본값을 읽지 못하면 해당 설정은 보존한다. raw JSON의 개별 Save는 입력 문서를 보존하며 다음 일반 저장에서 알려진 중복값을 정리한다. Specific이 글로벌과 같아져 제거되면 이후 글로벌 변경을 따라간다(동일값 고정 Override는 유지하지 않음).

dirtyEffects는 최소 표현으로 정리한 현재 효과/모든 기체 Override와 저장 기준의 차이로 매번 계산한다. 단순히 값을 한번 바꿨다는 이유로 수정 표시를 유지하지 않는다. Lua/JSON 초안도 Editor의 저장 텍스트와 같아지면 제거하며 CRLF/LF에 따른 편집기 내부 줄바꿈 정규화를 반영한다. 저장된 스크립트 기본값이 바뀌면 메타데이터 캐시를 갱신한다.

표시는 Default/Global/Specific 세 단계이며 각 행에 현재 편집 범위에 맞는 Reset to Default 또는 Reset to Global을 둔다. Power는 가로 한 줄(PWM 체크 시 ON/Period 노출), Outputs는 출력명 헤더와 목적지 셀 한 행으로 구성한다. 여러 목적지는 개행하여 표시하고 가로 폭을 넘는 출력은 스크롤한다.

### haptic_power 출력 사양 직접 사용

vibrate의 두 번째 인수는 기존 mode='constant'/'pwm'/'off' 사양 또는 haptic_power 객체다. mode가 없으면 필수 boolean pwm이 false일 때 Constant, true일 때 PWM으로 해석한다. strength는 정수 0–255, PWM이면 on_ms/period_ms를 기존 규칙으로 검증·양자화하며 duration_ms와 요청 교체 규칙도 동일하다. 명시적 mode는 pwm보다 우선한다. 전달 객체를 수정하지 않으며 파라미터 상속은 기존 configure 단계에서 완료된다. 예: `context:vibrate("Left Gear", parameters.Power, 150)`. 동적 파형 작성에 쓰는 기존 헬퍼 API는 변경하지 않는다.

Reset All은 선택 효과의 편집 프로필에 한정한다. Global은 enabled/parameters/outputs를 제거해 기본값을 상속하고, Specific은 해당 효과 Override 전체를 제거해 Global을 상속한다. 다른 효과와 Lua 소스는 변경하지 않는다.

## 분리된 Lua 인수와 진단 계약 (2026-11-11)

하위 호환 분기는 없다. init(context), update(context, telemetry, parameters)를 호출하고 self는 별도 implicit 인수다. context에는 메서드만 둔다. init: parameter/output/log. update: log/vibrate/now()/elapsed()/sample_is_new()/input_valid(). now/elapsed는 호출 기준 단조 ms, 첫 elapsed는 0이다. telemetry는 src/telemetry.h의 지원 SimVar 키 및 sample_id/received_ms/aircraft_key/sim_time_advanced/gears/msfs.paused다. gears[1..3]는 센터/좌/우, MSFS 인덱스 0..2와 대응한다. telemetry와 parameters를 context 안에 복제하지 않는다.

parameters 키는 등록 name의 각 공백 문자를 _로 치환한다. 결과는 ASCII Lua 식별자이며 예약어 금지, 동일 접근 키 중복은 init 오류다. 표시명과 JSON 저장 키는 원래 name이고 실행 인수에는 변환 키만 둔다. default→global→기체 병합 후 변환한다. 자동완성은 init 안의 리터럴 name/type 등록을 읽는다. 동적 계산된 등록 이름의 완성은 지원하지 않는다.

HID 틱은 PC 로컬 tick_ms(5..100, 기본20)이며 PWM 유효 on/period는 HID 루프에서 같은 tick으로 반올림해 순위를 비교한다. 반올림 결과 ON 또는 period=0이면 무출력 후보다. Tick 변경 시 기존 요청/테스트를 지우고 실행 세대를 올려 self를 재초기화한다. UI Telemetry 표시용 재전달은 실제 received_ms와 sample_id를 보존한다. 부가 필드의 비유한 값은 nil/Unavailable이며 필수 9개 입력의 비유한 값은 연결 오류다. 엔진 1..4 표준 SimVar를 구독하며 추력은 Pounds, rpm/N1/N2는 Percent다. 기체별 미지원 값이 0으로 반환되는 것과 실제 0은 SimConnect만으로 구별할 수 없다.

텔레메트리 완성 목록은 src/telemetry.h에 선언한 구독 계약이다. 연결/0값/정체로 지원 필드를 추정해 추가·제거하지 않는다. InputEvent 열거와 표준 SimVar 구독은 별도다. 프로젝트 open 시 effects 하위 디렉토리를 펼치고 아이콘 앞 체크(최상위 effects 제외)는 현재 편집 프로필의 활성화 상태를 제어한다.
