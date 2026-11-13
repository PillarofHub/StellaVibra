# MVP 계약 (형식 1)

## DCS 설치·수신 구현 (2026-11-14)

사용자가 자동 탐색 정상 동작을 확인하고 실제 스크립트 설치와 DCS 프로젝트 수신 구현을 요청했다. 이전 UI 한정 범위는 이번 요청으로 대체된다. 효과 Lua는 아직 구현하지 않는다.

상세 계약은 [DCS-INTEGRATION](docs/DCS-INTEGRATION.md)에 있다. Export.lua 고정 로더 한 줄 → Scripts/StellaVibra Telemetry/loader.lua → telemetry.lua 및 별도 config.lua, 설치 소유권/해시 manifest.json. 시작/메뉴에서 정확한 관리 행·파일 해시를 검사하며 Add/Update Script·Remove Script가 실제 동작한다. 로더 없음은 Add Scripts, 정상 로더/해시는 항상 Update Scripts, 수정된 Stella 행·무결성 오류는 확인 후 Restore Scripts. 행 또는 전용 폴더가 있으면 해시 불일치라도 Remove Scripts를 허용한다. 기본50ms/범위10–100ms, DCS 재시작 후 반영한다.

로컬 UDP 127.0.0.1:43891, 프로토콜1, JSON/16KiB 상한, 세션·순번·시각·권한·상태와 DCS 원본 입력. 별도 서버 없음. DCS 프로젝트 활성 동안만 수신하며 공백/세션·객체 교체에서 상태를 초기화한다. 신호 누락은 nil, 수명500ms와 송신 후 남은 수명을 함께 적용한다. 동일 스냅샷 재전달로 수신 시각/유효기한을 갱신하지 않는다.

Export.lua 전체 백업·원본 저장·전체 복원은 하지 않는다. 현재 파일의 Stella 관련 행만 교체/삭제하고 다른 바이트는 그대로 보존한다. 수정/중복/부분 설치는 확인 후 복구한다. Remove는 확인 후 전용 폴더 내용 전체를 삭제한다. UTF-16/32 등 비호환 인코딩·링크는 거부하며 파일별 원자 저장·실패 시 관리 파일만 롤백한다. 정확한 대상 행 판별·끝 개행·동시 편집 제한은 상세 계약 참조. config.lua는 코드 해시 검사가 아니라 전체 고정 형식과 정수10–100 범위를 직접 파싱한다. 숫자만 편집했으면 설정 창에서 현재값을 표시하며 정상 취급한다. 코드 Lua 두 개만 manifest 해시로 검증한다. Remove 후 Export.lua가 정확히0바이트면 삭제하며 BOM/공백/개행이 남으면 보존한다. 실제 사용자 Export.lua 설치/제거 및 런타임 테스트는 도구로 수행하지 않는다.

## DCS 후속 작업 — 입력 조사 후 효과 범위 논의 (2026-11-13)

최신 사용자 결정: MSFS와의 억지 일치 금지. Ground Roll, Airframe Shake, G Load, 선택적 Overspeed를 유지하고 High AoA/별도 Stall Buffet 및 Ground Thumps는 기본 효과에서 제외한다. Touchdown은 rod 기반 첫 상승을 먼저 구현·사용자 확인하고 문제가 있으면 통합 착지 출력으로 단순화한다. rod의 단위/갱신이 검증되었다는 뜻은 아니다. Engine RPM은 Shake 통합/공명 검토 후 보류 권고(최종 채택 미정), Startup/Shutdown·무장은 기존 후보로 남으며 새로 확정하지 않았다. 상세 최신 표는 docs/DCS-TELEMETRY.md의 후속 사용자 결정 절이다. 이전 후보 권고보다 이 결정이 우선한다.

## Explorer 파일 관리 (0.2.1 로컬 빌드)

New File 트리는 effects 자체를 선택 가능한 최상위 항목으로 표시하고 그 하위 폴더만 보여준다. 별도 루트 선택 버튼은 없다. 폴더 선택 시 effects/ 접두사 포함 경로를 표시하며 수기 입력은 접두사 생략도 허용한다. 생성 시 선두 effects/ 하나만 대소문자 무관 제거한 뒤 effects 루트 기준으로 해석하므로 중복 effects 디렉토리를 만들지 않는다. Project Explorer는 대소문자 구분 없는 이름 오름차순으로 고정한다. QFileSystemModel의 비동기 행 순서를 화면에 직접 쓰지 않고 동적 QSortFilterProxyModel로 표시 순서를 관리한다. 기존 타이머/내림차순·오름차순 강제 정렬은 제거한다. 트리 선택/확장은 프록시 인덱스로 변환하고 열기/메뉴/드래그는 FilePathRole의 실제 경로를 사용한다. GUI 검증은 사용자 확인 전이다.

effects 루트 우클릭: New Folder/New Script. 하위 폴더: 생성 두 개와 Rename/Delete. Lua: Rename/Delete. Move To는 제거하고 Explorer 내부 드래그로 Lua/하위 폴더를 다른 effects 폴더에 이동한다. 외부 드롭/루트 이동/파일 위 드롭은 거부한다. File → New File은 Save 위에 두고 effects 루트의 폴더 트리와 상대 파일 경로 입력을 제공한다. 폴더 클릭은 입력의 폴더 부분을 채우며 파일명은 유지한다. 모든 경로 조각을 검사한 뒤 없는 폴더를 생성하고 예제 Lua를 쓴다. 취소 시 생성 없음; 파일 쓰기 실패로 이미 생성한 빈 폴더가 남을 수 있다. 우클릭 생성은 이름 입력을 유지한다. raw JSON 초안 중 파일 변경 금지, .lua 자동 추가, 덮어쓰기/폴더 합치기 금지, effects 밖 이동/자기 하위 이동/링크·junction 경로 변경 금지를 유지한다.

명시적 이름 변경/이동은 효과 UUID와 Global/Specific 설정 및 열린 탭/초안/undo를 유지하고 저장된 구조 경로만 갱신한다. 이름 입력/삭제 확인 중 파일 추적을 보류하며 New File 경로 선택 중에는 정상 추적하되 생성 시 프로젝트를 재확인한다. 외부 파일 변경 추론은 별개다. 삭제는 내용·설정·초안 제거 확인 후 휴지통으로 보내며 실패 시 기본 No인 별도 영구 삭제 확인을 표시한다. Yes에서만 effects 내부 경계·하위 링크를 재검사하고 삭제한다. 부분 실패는 경고하고 파일 추적을 갱신한다. 휴지통 복원은 효과 설정 복원이 아니다. 새 Lua의 진동 호출 예제는 주석이며 기본 출력은 미배정이다.

현재 제품 기준은 사용자 지정 0.2.1이며 DCS는 아직 지원하지 않는다. 이번 작업은 로컬 커밋까지이며 태그·Release는 만들지 않는다. 아래는 사용자 요청으로 정해진 작업 경계이며, 효과 후보나 전송 계약의 구현 승인이 아니다.

- 사용자 정정: 효과 범위/대체안의 선택 전에 입력 제공 범위를 조사한다. 현재는 1단계 후보를 평가하기 위한 입력 조사이며 전달 계약/설치/효과 구현은 시작하지 않는다. 조사 결과는 [DCS-TELEMETRY](docs/DCS-TELEMETRY.md)에 기록한다.
- 하나의 DCS 프로젝트와 공통 Lua를 목표로 한다. 주 사용 기체는 지원 범위 선택 기준이 아니며, 기체별 조종석 인자 매핑을 필수 기반으로 삼지 않는다. 공통 API 존재와 모든 기체에서의 값 갱신 보장은 구분한다.
- DCS 원본 입력의 의미를 보존하며 다른 게임과 공통 텔레메트리로 정규화하지 않는다. 효과 판단·재무장·지속 시간·상태는 효과 Lua, 연결·수신·유효성·실행·파형·우선순위·HID는 C++ 책임이다.
- 기존 init/update/self, 이름 기반 출력, haptic_power 직접 전달, 요청 사양·기한 교체, 물리 모터별 단일 요청 선택을 유지한다. 실제 FFB 명령/힘을 읽거나 방향·저항을 진동으로 재현한다고 가정하지 않는다.
- Ground Roll/Touchdown은 MSFS 사용감을 목표로 한다. 압축이 없는 Touchdown 대체안, 기체 전체 접지를 바퀴별 출력에 사용하는 방식, Ground Thumps의 가상 이음매 모델은 아직 합의되지 않았다. 무장 출력은 사용자 희망에 따라 Stick을 우선 제안한다.
- 후보: Ground Roll, Touchdown, Ground Thumps, High AoA, Stall/Buffet, G Load, Airspeed, Engine RPM/Startup/Shutdown, Weapon Release/Launch, Gunfire. 필수 입력이 확인되지 않으면 지원으로 확정하지 않는다. 누락은 unavailable이며 측정된 0으로 채우지 않는다.
- 1단계 제안: 신뢰 가능한 접지 없이 고도/수직 속도만으로 자동 착륙을 판정하지 않는다. 접지는 있고 압축만 없을 때 고정 펄스 대체를 논의한다. RPM 경계는 시동/정지의 추정임을 구분하고 무장 수량 감소만으로 발사를 단정하지 않는다. 모두 사용자 합의 대기다.
- 설치 DCS 2.9.31.29434의 공식 Scripts/Export.lua, Sim_ControlAPI, FM 헤더/설정 및 실제 TelemFFB exporter를 대조했다. 공중 여부, shake, 속도/AoA/G, 엔진 RPM, 무장 잔량의 API는 확인됐다. rod 필드는 있으나 공통 압축 의미/범위는 미확정이다. 세부 근거·한계는 DCS-TELEMETRY를 따른다. 실게임 샘플/전 기체 동작 검증은 하지 않았다.
- 조사 후 제안: 통합 Ground Roll, Airframe Shake, High AoA/G Load/Airspeed/Engine RPM은 공통 API 기반 후보다. Touchdown 첫 압축 상승은 아직 확정 불가하며 통합 고정 펄스는 타협안이다. 시동/정지는 RPM 추정, 무장은 종류별 잔량 감소이며 실제 발사와 비상투하의 완전 구분은 불가하다. 기본 Ground Thumps의 포장/활주로 조건을 충족하는 일반 Export 신호는 확인하지 못했다. 이 제안들은 사용자 합의 전이다.
- localhost 전달 방식·포트·유효기간, 로더 한 줄의 경로/API, FPS 범위·반영 시점, Update Timing의 중계기 갱신 포함 여부는 미정이다. 기본 20 FPS 및 전용 설정 파일, 기존 exporter 보존은 사용자 요구로 유지한다.

대상 게임은 MSFS 2024 / MSFS 2020 / X-Plane 12 / War Thunder 중 프로젝트별 하나다. 플랫폼은 Windows 10/11 x64, C++20/MSVC, Qt Widgets 6.8.3 동적 링크, Lua 5.4.8 정적 링크다. Stella 소스는 MIT다. Qt 및 Microsoft 구성요소의 라이선스를 MIT로 재지정하지 않는다.

## 파일

프로젝트 `.sv`: `{"version":1,"id":"UUID","name":"Flight","game":"MSFS2024","effects":[{"id":"UUID","script":"effects/ground/ground_roll.lua","enabled":true,"parameters":{"Strength":20},"outputs":{"Left Gear":["Throttle:1"],"Right Gear":["Throttle:2"]}}]}`.

기체 `aircrafts/UUID.sv`: `{"version":1,"aircraft_key":"정확한 TITLE","effects":{"효과 UUID":{"enabled":false,"parameters":{"Strength":10},"outputs":{"Left Gear":[]}}}}`.
누락은 상속, false와 []는 명시값. 출력은 이름별 전체 교체. Reset to Global은 해당 키 삭제. 일치하는 기체 파일이 두 개면 실행 거부. MSFS 기체 키는 같은 샘플의 TITLE 문자열 전체다. X-Plane은 acf_descrip, War Thunder는 indicators.type이다. 이름을 다른 게임의 공통 기체명으로 변환하지 않는다.

로컬 파일은 실행 파일과 같은 디렉토리의 `StellaVibra.pref`(UTF-8 JSON): `{"version":1,"last_project":"절대 경로","recent_projects":["절대 경로"],"motors":{"장치키/Left":{"slots":["Throttle:1"],"manual":false}},"geometry":"base64","layout":"base64"}`. 경로는 현재 작업 디렉토리가 아닌 applicationDirPath 기준이다. 기존 AppData/레지스트리 읽기·자동 이전·저장 실패 시 대체 저장은 하지 않는다. 파일이 없으면 기본값이며 첫 설정 저장 시 QSaveFile로 생성한다. 쓰기 불가 시 해당 경로의 저장 오류를 표시한다. 물리 식별은 VID/PID+일련번호. 일련번호가 없거나 중복이면 경로 기반 임시 키로 표시하고 자동 배정하지 않으며 매 앱 세션 수동 확인이 필요하다. 의도적 해제는 manual=true와 빈 slots로 보존한다. 예약된 매핑도 자동 배정 시 점유로 센다. 슬롯은 Yoke/Stick/Throttle/Pedal/Wheel 각각 :1–:4다.

JSON 상위 버전·잘못된 자료형·경로 이탈·중복 ID는 거부한다. 프로젝트 기준 상대 Lua 경로만 허용한다. QSaveFile로 파일별 원자 교체하며 프로젝트 전체 다중 파일 트랜잭션은 아니다. 미해결 파라미터/포트 키는 보존하고 UI에 표시한다.

## Lua

파일은 `local effect = {}; function effect:init(context) ... end; function effect:update(context, telemetry, parameters) ... end; return effect` 형식이다. self는 해당 인스턴스의 테이블이다. init에서만 `context:parameter{name=string,type="boolean"|"int"|"float"|"haptic_power"|"enum",default=...}`를 등록한다. int/float은 min/max/step이 필수이며 값은 유한하고 범위 안이어야 한다. int는 기본값·범위·step·저장값 모두 정수다. type은 필수다. 이름 중복은 오류다. `context:output(name, default_slots)`은 이름 문자열 포트와 선택적 기본 논리 슬롯 배열을 등록한다. default_slots 생략/nil/빈 배열은 미배정이며, 1부터 연속된 최대20개 유효 슬롯 문자열·중복 없음만 허용한다.

update의 `parameters`는 default→global→현재 기체 합성값이다. `context:log(text)`, `context:vibrate(name,constant(strength),duration_ms)`, `context:vibrate(name,pwm{strength=...,on_ms=...,period_ms=...},duration_ms)`, `context:vibrate(name,off())`를 제공한다. 세기는 정수 0–255, 기간은 0초과 60000ms 이하, PWM은 0≤ON≤period≤60000, period≥20ms다. OFF에는 기간이 없다. 정상 콜백이 끝난 뒤 명령 전체를 적용하며 오류 시 해당 효과 출력을 해제한다.

Lua 라이브러리는 base/math/string/table/utf8만 허용하고 파일·프로세스·네트워크·package/debug/coroutine/load/pcall/xpcall/setmetatable/getmetatable을 제거한다. Lua는 C++로 컴파일해 오류 시 호스트 객체도 예외 해제한다. 인스턴스 Lua 할당 메모리 8MiB, 콜백 200000명령 또는 20ms(1000명령마다 검사하며 네이티브 라이브러리 호출의 하드 실시간 선점은 아님), 소스 256KiB, 등록 각각 64개, 콜백 출력 128개와 로그 8개로 제한한다. top-level과 init도 보호 호출 안에서 실행한다.

## 입력과 실행

UI, Lua/게임 연결 실행 스레드, HID 스레드를 분리한다. SimConnect 호출은 실행 스레드에 한정한다. 입력 프레임은 최대 64개씩 직렬 처리한다. backlog 초과 시 연결을 끊고 출력을 정리한다. 새 입력은 수신 직후 update한다. 새 입력 없이 maximum_update_period_ms(5..500, 기본100)가 경과하면 최신 스냅샷을 재전달하되 sample_id와 received_ms를 변경하지 않는다. context:now()/elapsed()는 호스트 steady_clock ms다. MSFS 입력 age는 context:now()-telemetry.received_ms이며 context:input_valid()는 수신 후 500ms 이내이고 sim 시간이 최근 500ms 안에 전진했으며 pause가 아닐 때만 true다. 기체 변경·시간 역행·pause·오류·Stop은 출력과 효과 상태를 초기화한다. 가속 시 파형과 Touchdown 지속 시간은 벽시계 기준이다.

MSFS telemetry: `sample_id, received_ms, sim_time_s, aircraft_key, ground_speed_knots, on_ground, gears={ {contact=bool,animation_percent=number}, ... }`, gears 순서는 Center/Left/Right다. MSFS 전용 값은 `msfs={paused=bool}`. 누락값은 nil이며 첫 샘플 전 유효하지 않다. 중복 시각도 실제 패킷이면 context:sample_is_new()=true, `sim_time_advanced=false`; 재전달은 둘 다 false다. 완전 중복 패킷은 폐기해 수신 시각을 연장하지 않는다.

## 출력

HID 기본 20ms, 실행 폴더별 tick_ms=5..100ms. 기간은 update now+duration으로 교체한다. PWM 시간은 floor(ms/tick_ms+0.5)*tick_ms, 유효값으로 순위 비교. PWM→PWM은 만료되지 않은 기존 위상 유지, 다른 모드→PWM은 ON부터 시작. 요청 선택은 Constant 우선, 세기 내림차순, PWM 주기 오름차순·ON 내림차순. 완전 동률은 현재 승자 유지, 최초는 효과 UUID/포트 이름 사전순. 선택 후 파형을 계산하며 OFF 구간을 다른 후보로 채우지 않는다.

실행기 heartbeat 250ms 초과, 입력 500ms 만료, Stop 시 HID 스레드가 출력 해제. 정상 heartbeat는 기존 예약을 연장하지 않는다. generation 변경 후 낡은 명령은 버린다. 테스트는 모터 하나, 명시적 Start, 최대 5000ms이며 그 모터 게임 출력을 제외한다. 창을 열기만 하면 출력하지 않는다. 전역 Stop은 테스트도 종료한다.

## 효과

MSFS 배포 템플릿은 `effects/MSFS2024/ground/ground_roll.lua`, `effects/MSFS2024/ground/touchdown.lua`이며 새 프로젝트에는 게임 디렉토리 없이 `effects/ground/...`로 복사한다. Ground Roll은 속도/접촉 게이트와 짧은 재요청을 Lua에서 수행한다. Touchdown은 바퀴마다 첫 airborne 관측 후 재무장한다. 접촉 상승에서 시작하여 첫 평탄/하강한 새로운 sim-time 샘플에서 첫 상승 시간×배수를 min/max로 제한해 최초 접촉 벽시계 기준 기한을 확정한다. 이후 피크는 무시한다. 중복 sim-time 접촉 변화는 수용하지만 상승 종료 근거로 사용하지 않는다. 압축 애니메이션은 힘이 아니며 미지원/정적인 기체는 최소 펄스를 사용한다. 확정 전에는 min과 관측된 상승 시간에 맞춰 스크립트가 재요청하며 호스트는 연장하지 않는다.

## 연동 근거

- [MSFS 2024 SimConnect](https://docs.flightsimulator.com/msfs2024/retail/programming-apis/simconnect/simconnect-sdk/): 동적 로드로 Open/Close/AddToDataDefinition/RequestDataOnSimObject/GetNextDispatch/SubscribeToSystemEvent 사용. SDK 미설치로 공식 공개 ABI 최소 선언을 사용한다. 2024 DLL은 bin/simconnect/MSFS2024/SimConnect.dll, 2020 DLL은 bin/simconnect/MSFS2020/SimConnect.dll만 로드한다. 공식 SDK 클라이언트를 로컬 Release/dist에 복사한다. 공개 재배포 권리 확인은 SIMCONNECT-LICENSING.md를 따른다.
- [수신 ID](https://docs.flightsimulator.com/msfs2024/retail/programming-apis/simconnect/api-reference/structures-and-enumerations/simconnect_recv_id/): EXCEPTION=1. Aurora의 5 선언을 그대로 복사하지 않는다. [자료형 정의](https://docs.flightsimulator.com/msfs2024/retail/programming-apis/simconnect/api-reference/structures-and-enumerations/simconnect_datatype/)에 따라 STRING256=9이며 Aurora의 12 선언도 사용하지 않는다.
- [기어 변수](https://docs.flightsimulator.com/msfs2024/retail/programming-apis/simvars/aircraft-simvars/aircraft-brake-and-landing-gear-variables/): GEAR IS ON GROUND:0/1/2 Bool, GEAR ANIMATION POSITION:0/1/2 Percent. ABSOLUTE TIME Seconds, GROUND VELOCITY Knots, SIM ON GROUND Bool, TITLE STRING256.
- HID: Aurora `src/hid/protocol.rs` 및 HANDOFF. VID 4098/PID B970은 Left=(04,0E), Right=(08,10). `[reportID,ident,B9,0,0,03,49,address,strength,0,0,0,0,0]`, B970은 reportID=02를 descriptor에서 확인한다. 모든 지원 장치는 길이≥14, UsagePage=1/Usage=4를 요구한다. 다른 모델의 목록·형식은 아래 확장 계약을 따른다. 비동기 WriteFile 완료/실패를 확인하고 제한 시간 초과 시 취소한다.

## 편집·파일 추적·자동 실행

File에 New/Open/Open Recent/Save/Save All/Close File/Close Project를 둔다. 시작 시 last_project만 복원하며 파일이 없으면 프로젝트 없는 상태다. Save는 현재 스크립트와 해당 효과의 글로벌/모든 기체 설정만 저장한다. 다른 효과의 초안은 저장하지 않는다. Save All은 전체 저장한다. 스크립트는 기체 공통이며 Parameters 상단의 편집 기체 선택은 코드 저장에 영향을 주지 않는다.

파라미터는 즉시 합성·적용하며 Lua self를 유지한다. 코드 저장 또는 외부 파일 내용 변경은 해당 효과만 검증·교체한다. 각 효과의 모든 프로필을 검증하고 실패하면 그 효과의 이전 유효 버전을 유지한다. 다른 효과의 갱신을 막지 않는다. 코드 초안은 실행하지 않는다. 출력 경로/활성화 변경은 해당 요청을 정리한다. PWM 설정 변경은 활성 PWM 위상을 보존한다.

effects 하위 Lua를 1초마다 추적한다. 누락 경로와 새 경로가 같은 파일명으로 각각 유일하게 대응되면 ID·설정을 이전한다. 대응되지 않는 누락 효과와 모든 기체 Override는 삭제한다. 새 경로는 새 ID, enabled=true, 빈 parameters/outputs(스크립트 파라미터·출력 기본값 상속)로 등록한다. 구조 변경만 자동 저장하고 다른 효과의 파라미터 초안은 저장하지 않는다. 실제 파일 목록은 QFileSystemModel로 갱신한다. 프로젝트 JSON의 외부 동시 편집 병합은 제공하지 않는다. 삭제 파일의 미저장 코드 버퍼는 보존하지만 효과 설정은 삭제한다. 사용자가 버퍼를 Save하여 파일을 재생성하면 새 효과/기본값으로 등록된다.

프로젝트가 선택한 MSFS는 미연결 시 2초마다 재시도한다. 연결 중에는 [RequestSystemState](https://docs.flightsimulator.com/html/Programming_Tools/SimConnect/API_Reference/General/SimConnect_RequestSystemState.htm)의 Sim 요청을 2초 간격으로 보내고 응답이 5초간 없으면 재연결한다. 이 응답은 텔레메트리 수신/유효 시각을 바꾸지 않는다. HID는 1초마다 재검색한다. 기존 HID 핸들은 유지한다. 실행은 기본 활성 상태이며 유효한 프로젝트·입력·매핑·효과 요청이 있을 때만 출력한다. Stop All은 요청/테스트를 해제하고 Resume All 전까지 게임 출력을 차단한다. 프로젝트 전환은 출력을 정리하되 전체 중지 선택은 유지한다. 테스트 창 열기만으로 새 테스트 출력을 만들지 않는다.

## haptic_power 파라미터

```lua
context:parameter {
    name="Power", type="haptic_power",
    default={strength=20, pwm=false, on_ms=20, period_ms=80},
    min=0, max=255
}
```

default 테이블은 필수. 테이블 내 생략값은 strength=0, pwm=false, on_ms=20, period_ms=80이다. min/max는 선택이며 0/255가 기본, 정수 0..255 안으로 제한한다. strength는 지정 범위 내 정수, PWM은 boolean, ON/period는 정수 ms이며 0≤ON≤period, 20≤period≤60000이다. UI는 강도 슬라이더+직접 숫자 입력, PWM 체크, 체크 시 ON/period 입력을 표시한다. 실행 시간은 설정한 HID tick 반올림 규칙을 따른다. 일반 숫자도 슬라이더+직접 입력이며 step은 슬라이더/증감 간격이다(직접 입력값을 step 배수로 강제하지 않는다). 슬라이더 분할은 최대 1,000,000이다.

저장 및 parameters.Power는 `{strength:20,pwm:false,on_ms:20,period_ms:80}` 객체다. 복합 파라미터 하나가 상속·Override의 단위이며 내부 필드별 부분 상속은 하지 않는다. Reset All to Global은 선택 효과의 현재 기체 Override 전체를 지운다. 예제는 이 객체를 vibrate에 직접 전달하거나 constant/pwm 사양을 만들며 파형 선택·게임 판단은 Lua에 남긴다.

## 도킹 UI·HID 확장 (2026-11-10)

View 메뉴의 Project Explorer / Effect Control / Motor Monitor / Debug Output은 각 QDockWidget 표시 상태를 제어한다. Effect Control에 Parameters/Script Editor를 함께 둔다. 상태는 StellaVibra.pref의 layout에 보존하며 layout_version=4는 새 기본 크기 배치의 적용 여부다. 숨김은 실행 중지를 의미하지 않는다. Monitor는 연결 모터만 Motor/Slots/HID 세 열로 표시한다. HID는 마지막 성공 전송 강도이며 20ms UI 갱신은 하드 실시간/모든 펄스 관측 보장이 아니다. 연결 해제 후 PC 매핑 예약은 유지한다.

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

프로젝트 enabled 누락은 true다. 글로벌 파라미터 누락은 Lua default, 출력 이름 누락은 Lua output 등록 기본값(생략 시 빈 목록)을 상속한다. Specific 누락은 global/default 합성값을 상속한다. Global에서 default와 동일한 항목, Specific에서 현재 global 합성과 동일한 항목은 생략한다. 출력 배열과 haptic_power 객체는 전체 값 단위로 비교한다. enabled=false 또는 outputs=[]도 상위와 다르면 명시값으로 보존한다. 빈 parameters/outputs 그룹, 빈 효과 Override는 제거한다. 알 수 없는 파라미터/출력은 기본값을 추정해 제거하지 않는다.

UI 설정 조절/메타데이터 확인 시 최소 표현으로 정리하고 일반 Save/Save All에서 파일에 반영한다. 스크립트 오류로 기본값을 읽지 못하면 해당 설정은 보존한다. raw JSON의 개별 Save는 입력 문서를 보존하며 다음 일반 저장에서 알려진 중복값을 정리한다. Specific이 글로벌과 같아져 제거되면 이후 글로벌 변경을 따라간다(동일값 고정 Override는 유지하지 않음).

dirtyEffects는 최소 표현으로 정리한 현재 효과/모든 기체 Override와 저장 기준의 차이로 매번 계산한다. 단순히 값을 한번 바꿨다는 이유로 수정 표시를 유지하지 않는다. Lua/JSON 초안도 Editor의 저장 텍스트와 같아지면 제거하며 CRLF/LF에 따른 편집기 내부 줄바꿈 정규화를 반영한다. 저장된 스크립트 기본값이 바뀌면 메타데이터 캐시를 갱신한다.

표시는 Default/Global/Specific 세 단계이며 각 행에 현재 편집 범위에 맞는 Reset to Default 또는 Reset to Global을 둔다. Power는 가로 한 줄(PWM 체크 시 ON/Period 노출), Outputs는 출력명 헤더와 목적지 셀 한 행으로 구성한다. 여러 목적지는 개행하여 표시하고 가로 폭을 넘는 출력은 스크롤한다.

### haptic_power 출력 사양 직접 사용

vibrate의 두 번째 인수는 기존 mode='constant'/'pwm'/'off' 사양 또는 haptic_power 객체다. mode가 없으면 필수 boolean pwm이 false일 때 Constant, true일 때 PWM으로 해석한다. strength는 정수 0–255, PWM이면 on_ms/period_ms를 기존 규칙으로 검증·양자화하며 duration_ms와 요청 교체 규칙도 동일하다. 명시적 mode는 pwm보다 우선한다. 전달 객체를 수정하지 않으며 파라미터 상속은 기존 configure 단계에서 완료된다. 예: `context:vibrate("Left Gear", parameters.Power, 100)`. 동적 파형 작성에 쓰는 기존 헬퍼 API는 변경하지 않는다.

Reset All은 선택 효과의 편집 프로필에 한정한다. Global은 enabled/parameters/outputs를 제거해 기본값을 상속하고, Specific은 해당 효과 Override 전체를 제거해 Global을 상속한다. 다른 효과와 Lua 소스는 변경하지 않는다.

## 분리된 Lua 인수와 진단 계약 (2026-11-11)

하위 호환 분기는 없다. init(context), update(context, telemetry, parameters)를 호출하고 self는 별도 implicit 인수다. context에는 메서드만 둔다. init: parameter/output/log. update: log/vibrate/now()/elapsed()/sample_is_new()/input_valid(). now/elapsed는 호출 기준 단조 ms, 첫 elapsed는 0이다. telemetry는 src/telemetry.h의 지원 SimVar 키 및 sample_id/received_ms/aircraft_key/sim_time_advanced/gears/msfs.paused다. gears[1..3]는 센터/좌/우, MSFS 인덱스 0..2와 대응한다. telemetry와 parameters를 context 안에 복제하지 않는다.

parameters 키는 등록 name의 각 공백 문자를 _로 치환한다. 결과는 ASCII Lua 식별자이며 예약어 금지, 동일 접근 키 중복은 init 오류다. 표시명과 JSON 저장 키는 원래 name이고 실행 인수에는 변환 키만 둔다. default→global→기체 병합 후 변환한다. 자동완성은 init 안의 리터럴 name/type 등록을 읽는다. 동적 계산된 등록 이름의 완성은 지원하지 않는다.

HID 틱은 실행 폴더별 tick_ms(5..100, 기본20)이며 PWM 유효 on/period는 HID 루프에서 같은 tick으로 반올림해 순위를 비교한다. 반올림 결과 ON 또는 period=0이면 무출력 후보다. Tick 변경 시 기존 요청/테스트를 지우고 실행 세대를 올려 self를 재초기화한다. UI Telemetry 표시용 재전달은 실제 received_ms와 sample_id를 보존한다. 부가 필드의 비유한 값은 nil/Unavailable이며 필수 9개 입력의 비유한 값은 연결 오류다. 엔진 1..4 표준 SimVar를 구독하며 추력은 Pounds, rpm/N1/N2는 Percent다. 기체별 미지원 값이 0으로 반환되는 것과 실제 0은 SimConnect만으로 구별할 수 없다.

MSFS 텔레메트리 완성 목록은 src/telemetry.h에 선언한 구독 계약이다. 연결/0값/정체로 지원 필드를 추정해 추가·제거하지 않는다. InputEvent 열거와 표준 SimVar 구독은 별도다. 프로젝트 open 시 effects 하위 디렉토리를 펼치고 아이콘 앞 체크(최상위 effects 제외)는 현재 편집 프로필의 활성화 상태를 제어한다.

## 게임별 입력 계약 확장

- `game`: MSFS2024 / MSFS2020 / XPlane12 / WarThunder. 실행 연결은 현재 프로젝트의 게임 하나다. 공통 텔레메트리 필드로 정규화하지 않는다. 내부 Telemetry의 sequence/received/advancedAt/aircraft/game은 실행·만료·기체 Override 선택을 위한 호스트 메타데이터다. X-Plane/WT Lua 객체에는 MSFS의 gears/msfs/sample_id 등을 주입하지 않는다.
- MSFS: 기존 48개에 surface_type(enum), g_force(G), acceleration_body_x/y/z(ft/s²), incidence_alpha/stall_alpha(rad), gear_total_pct_extended(%), brake_left/right_position(Position 0..32768)를 추가한다. 59개 수치/bool+TITLE이며 선언 순서로 등록·디코딩한다. 2020도 동일 표준 변수 계약을 사용하되 별도 공식 DLL을 로드한다. 실제 기체 값의 갱신은 미검증이다.
- X-Plane: `game_catalog.h`의 86개 원본 dataref 경로를 문자열 키로 제공한다. RREF 숫자는 float32를 JSON number로 전달하고 bool 의미도 0/1 숫자를 유지한다. 빠른 값은 500ms, 1Hz 값은 2500ms 후 Lua에서 제외한다. network_time_sec 전진과 pause=0, 최신 acf_descrip 40바이트 수신을 요구한다. 누락 필드는 nil이며 0으로 채우지 않는다. 2초마다 10개씩 나눠 재구독하고 종료 시 frequency=0으로 해제한다. 최근 응답 1500ms 없으면 연결 해제로 표시하며 모터 입력은 더 짧은 500ms로 만료한다.
- WT: 원본 `/state`와 `/indicators`를 한 폴링 주기의 두 객체로 수신한다. 둘 다 HTTP 200·유효 JSON이어야 샘플을 만든다. 350ms 제한, 실패 후 1초 재시도, 정상 시 최대 20Hz. state.valid=true 및 indicators.type이 필요하며 indicators.valid가 false면 무효다. 응답에 없는 필드를 만들어 넣지 않는다. 호스트 sim clock을 게임 시간으로 위장하지 않는다. 신뢰할 수 있는 게임 시각/pause 입력이 없으므로 두 JSON이 그대로면 진행 시각을 갱신하지 않고 500ms 후 출력을 멈춘다. 따라서 실제 비행 중 값이 완전히 일정한 경우에도 보수적으로 중지될 수 있다. HTTP 새 응답의 received는 갱신되지만 advancedAt은 변화 시에만 갱신한다. 호스트의 스냅샷 재전달은 둘 다 유지한다.
- 모터 만료·generation·Stop·Lua 오류 격리는 기존 계약과 같다. 게임을 바꾸거나 닫으면 연결·샘플·출력 요청을 정리한다. UI 상태/자동완성은 선택한 게임에 종속되며 다른 게임 필드를 보여주지 않는다.
- New Project는 존재하지 않는 절대 디렉토리를 생성한다. 기존 빈 디렉토리는 확인 없이 사용하고, 내용이 있는 디렉토리만 기본 No인 확인창 후 `<폴더명>.sv`와 이름이 겹치는 기본 Lua만 파일별 원자 교체한다. 다른 파일과 기존 aircrafts는 보존하며 기존 effects 내 Lua는 추적 대상이다. 디렉토리 전체 삭제나 프로젝트 전체 트랜잭션은 하지 않는다.

## 타이밍 설정

Settings → Set Timings에서 HID tick(5..100ms, 기본20)과 Maximum update period(5..500ms, 기본100)를 함께 편집한다. Reset to Defaults는 두 입력을 초기화하고 OK에서 적용·pref 저장, Cancel은 변경하지 않는다. 설명문은 표시하지 않는다. maximum_update_period_ms 누락 시100을 사용하며 불러온 값은 허용 범위로 제한한다. 실행 스레드는 atomic으로 읽는다. 최대 간격은 새 텔레메트리를 기다리지 않고 기존 스냅샷을 다시 처리할 목표 주기이며 OS/5ms 실행 루프에 따른 지연이 있을 수 있다. 새 샘플은 즉시 처리하며 입력 만료500ms·heartbeat250ms·기존 출력 기한은 바꾸지 않는다. 최대 간격만 변경할 때 Lua 상태·현재 요청은 초기화하지 않는다.

출력 기본값은 LuaEffect 메타데이터의 outputDefaults에서 실행 configure와 편집기 기본값/최소 저장/리셋에 동일하게 적용한다. C++ 새 프로젝트 생성의 Airframe/Engine/Left Gear/Right Gear 이름별 매핑은 제거한다. 새 `.sv`에는 기본 출력 매핑을 기록하지 않는다. 명시적 outputs[name]=[]는 상속을 차단하고, 배열을 통째로 합성한다. UI의 목적지 선택창도 기본값까지 합성한 채널을 선택 상태로 표시한다.

Browse는 getExistingDirectory 대신 폴더 전용 QFileSystemModel/QTreeView와 경로 입력을 사용한다. Select Folder에서 상대 경로는 선택 폴더 기준으로 해석하고 QDir::mkpath로 없는 경로를 생성한다. Windows 구분자는 정규화한다. Browse Cancel은 생성하지 않으며 Select Folder 후 New Project Cancel은 이미 만든 폴더를 삭제하지 않는다. 빈 폴더 검사는 AllEntries/Hidden/System/NoDotAndDotDot를 사용한다. Qt 파일 대화상자 전역 이력 저장은 추가하지 않는다.

진단 파일은 EXE 옆 diagnostics에 세션별 log/dmp로 저장하며 Windows 설정에 덤프 수집 레지스트리를 만들지 않는다. 로그는2MiB 회전(.previous1개), 덤프는 처리되지 않은 예외에서 최선의 시도이며 생성 보장은 없다. SimConnect 모듈은 PIN으로 프로세스 수명까지 유지하고 연결 핸들만 재연결 때 정리한다. 공개된 Windows API 의미에 따른 수명 보완이며 실제 크래시의 원인 확정과 구분한다.

창 배치: 전체 창 크기 변경 시 마지막 사용자 조정 도크 비율을 유지한다(부동 창 제외). 기본 좌우 1:4:1, 본문/Debug Output 3:1, Parameters/Script 1:1. 최소 크기는 0에 가깝게 완화하고 제목줄·분할선·반올림에 따른 근사를 허용한다. 기존 저장 배치는 보존하며 View → Reset Default Layout으로 새 기본값을 적용한다.

## 채널 수·선택 파라미터·기본 효과 (2026-11-12)

실행 폴더 pref의 channels 객체에 Yoke/Stick/Throttle/Pedal 수를 0..4로 저장한다. 미지정 기본값은 기존과 같은4, Rudder UI의 저장 키는 Pedal이다. 기존 Wheel 카테고리4개는 유지한다. Lua output 기본값은 count=0이면 제거하고, count>0이면 (번호-1)%count+1로 순환 배정하며 중복은 제거한다. configure·UI·최소 저장 기준에 동일하게 적용하고 .sv에 자동 복사하지 않는다. 명시적으로 저장된 프로젝트/기체 출력과 물리 모터 매핑은 재작성하지 않는다. 설정 변경은 다음 실행 관리 업데이트에서 반영하며 바뀐 경로의 기존 요청은 제거한다.

enum 파라미터는 문자열 default와 1..64개의 중복 없는 문자열 choices 연속 배열을 요구한다. default 및 저장값은 choices 안에 있어야 한다. UI는 드롭다운, Lua/JSON 값은 문자열이며 기존 상속·리셋·최소 저장 규칙을 사용한다. 예: context:parameter{name="Ground type",type="enum",default="Concrete",choices={"Concrete","Asphalt","Concrete or asphalt"}}.

기본 스크립트의 일반 갱신 요청은150ms에서100ms로 줄인다. 이 값은 예제 정책이며 C++ API 강제값이 아니다. Touchdown은200/2000ms, multiplier2; Ground Roll은strength1/PWM ON/minimum0.1knots/step0.1. 나머지 효과의 기본 PWM은 OFF. Thrust/RPM/Power/Shutdown은 상속된 haptic_power를 복사한 뒤 strength만 변경하고 그대로 vibrate에 전달한다. MSFS/X-Plane Overspeed는 게임 한계값만 사용한다. WT는 게임 한계 입력이 없어 기존 수동 한계를 유지한다.

Ground Thumps는3개 기어의 접지 상태와 이동 거리 적분으로 가상 슬랩 경계 통과 시에만 요청한다. 기본4.6m, pulse100ms, MSFS 속도0.1..160knots(X-Plane0.1..82m/s). 각 바퀴의 실제 좌표/휠베이스는 없으므로 실제 이음매 통과 시각을 측정하지 않는다. 첫 샘플/시간 역행/0.5초 초과 간격/슬랩 길이 변경은 거리 기준을 재설정하며 이전 경계를 재생하지 않는다. 재전달 프레임은 거리·기한을 갱신하지 않는다. 펄스는 예상 벽시계 경계 간격의 절반 이하로 제한하고 다음 펄스까지 최소 동일 길이 휴지기를 둔다. HID tick보다 짧은 펄스/간격의 체감은 보장하지 않는다. 다른 효과의 선택 요청이 계속되면 모터의 전체 출력은 연속일 수 있다.

MSFS는 on_any_runway(ON ANY RUNWAY, Bool)를 추가 구독하고 선택한 surface_type(Concrete0/Asphalt4)과 함께 검사한다. X-Plane은 sim/flightmodel/ground/surface_texture_type(Concrete2/Asphalt3)을 추가 구독하되 활주로와 유도로/계류장 구분은 하지 못한다. War Thunder는 확인된 바퀴/노면 입력이 없어 Ground Thumps를 추가하지 않는다.

배포 루트의 StellaVibra.exe는 정적 CRT로 빌드한 Win32 런처이며 같은 루트/bin/StellaVibra.exe를 실행하고 인수를 전달한다. 실제 앱은 부모 배포 루트를 기준으로 pref/effects/licenses/sources/docs/diagnostics를 사용한다. Qt/MSVC DLL은 bin, Qt plugins는 bin/plugins(qt.conf), SimConnect DLL은 bin/simconnect/<게임>/에 있다. PATH·레지스트리·작업 디렉토리에 의존하지 않는다. 루트 런처와 bin 전체를 함께 이동해야 한다.
