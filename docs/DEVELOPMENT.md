# Stella Vibra 내부 개발 기준 (0.2.0)

<p align="center">
  <img src="../assets/stella-vibra-logo.png" alt="Stella Vibra — yellow star and sky-blue vibration marks" width="720">
</p>

Windows용 Lua 기반 햅틱 효과 편집·실행 도구. 제품 UI는 영어, 협업 문서는 한국어로 작성한다. 첫 MVP의 실행 코드와 Release 빌드가 구현되어 있다. 기존 UI 동작은 사용자가 확인했다. 이번 게임 확장의 GUI·게임 연결·하드웨어 동작은 아직 검증하지 않았다.

이 문서는 기존 한국어 README를 옮긴 내부 개발 기준이며 일부 항목에는 구현 과정의 이력이 포함되어 있다. 사용자용 문서는 ../README.md와 MANUAL.md, 현재 릴리스 범위는 RELEASE-NOTES-0.2.0.md를 따른다. 이전 --ui-check 임시 진단 경로는 0.2.0 릴리스 정리에서 제거했다. 작업 상태는 HANDOFF.md, 작업 순서와 새 채팅 프롬프트는 WORKFLOWS.md, 작업 제한은 AGENTS.md를 따른다. 세부 파일 스키마와 입력 필드는 CONTRACTS.md를 따른다.

## 기술과 MVP 범위

- C++20, Qt Widgets, CMake, MSVC, Windows 10/11 x64, 임베디드 Lua.
- Win32로 Windows/HID를 연동한다. DirectX는 필요할 때 사용하며 MVP UI에 별도 렌더러를 만들지 않는다.
- EXE와 필요한 DLL·플러그인을 동봉하는 포터블 폴더 배포. .NET 설치를 요구하지 않는다. Qt 6.8.3, Lua 5.4.8. 라이선스·배포 조건은 THIRD-PARTY-NOTICES.md를 따른다.
- 프로젝트별 MSFS 2024 / MSFS 2020 / X-Plane 12 / War Thunder 연결과 게임별 기본 Lua를 제공한다. 동시에 여러 게임을 연결하지 않는다.
- 프로젝트·기체 프로필, Lua 편집과 자동 파라미터 UI, 논리 출력 매핑, 출력 모니터, 모터 테스트 UI, 로그를 포함한다.
- 완전한 IDE/LSP/스텝 디버거, 다중 게임 동시 연결, 범용 플러그인 시스템, 스크립트 패키지 관리자, 녹화·리플레이, 장치별 출력 보정은 제외한다.
- Aurora Vibra는 읽기 전용 참고 구현이다. GUI·게임·하드웨어 검증과 모터 구동은 사용자가 수행한다.

## UI

| 위치 | 화면 | 기능 |
| --- | --- | --- |
| 상단 | File / Edit / View / Settings / Help 메뉴 | 프로젝트·저장 / Undo·Redo / 다섯 도킹 창 표시·숨기기 |
| 좌측 | Project Explorer | 단일 폴더 트리, Lua/하위 효과 폴더 아이콘 앞 활성화 체크박스, 접기·펼치기, 오류 표시 |
| 중앙 위 | Parameters | Global/Specific Aircraft 선택, 슬라이더·수치 입력, 출력 목적지, 모드별 개별/전체 Reset |
| 중앙 아래 | Script Editor | 파일별 탭, Lua/JSON 편집, 행 번호, 문법 강조, 검색, 저장 시 적용 |
| 우측 | Motor Monitor | 슬롯순 모터 목록, Slots 셀 매핑, HID 셀 테스트, Stop All/Resume All |
| 우측 아래 | Telemetry | 지원 SimVar 값·단위·Live/Paused/Stale 상태 |
| 하단 | Debug Output | 효과별 로그·오류, 필터, Clear, 오류 위치 이동 |
| 최하단 | Status Bar | 게임·장치 연결과 실행 상태 |

Project Explorer, Effect Control, Motor Monitor, Telemetry, Debug Output은 도킹 창이며 View 메뉴에서 각각 표시·숨기기를 바꾼다. Effect Control 최상단 파일 탭 아래에 Parameters/Script Editor를 위아래로 묶는다. 두 영역 모두 선택한 파일에 속하며 분할선을 끌어 높이를 조절한다. 창의 X로 닫아도 View에서 다시 열 수 있고, 창을 숨겨도 실행은 중지되지 않는다. 창 배치는 로컬에 저장한다. 현재 출력에 적용되는 기체와 편집 중인 프로필을 분리한다. 프로젝트에서 선택한 게임·지원 HID를 자동 연결하고 유효한 텔레메트리와 활성 효과가 있으면 자동 실행한다. Motor Monitor의 Stop All/Resume All이 전체 출력을 제어한다. 연결 자체나 테스트 창 열기는 진동 명령을 생성하지 않는다. 이전의 시작 시 Stopped 규칙은 이번 자동 실행 요청으로 변경되었다.

## 프로젝트와 설정

```text
My Flight Project/
  My Flight Project.sv
  effects/
    ground/
      ground_roll.lua
      touchdown.lua
  aircrafts/
    c172.sv
```

.sv 내부 형식은 JSON이다. 경로는 프로젝트 기준 상대 경로로 저장한다.

| 저장 위치 | 내용 |
| --- | --- |
| 프로젝트명.sv | 형식 버전, 프로젝트 ID·이름, 게임 종류, 효과 인스턴스 목록 |
| 효과 인스턴스 항목 | 고유 ID, Lua 경로, 활성화, 글로벌 파라미터, 출력 이름별 논리 목적지 목록 |
| aircrafts/*.sv | 형식 버전, 기체 식별 정보, 효과 인스턴스 ID별 Override |
| 프로그램 로컬 설정 | 물리 모터와 논리 슬롯의 매핑, 장치 식별·미배정 기록, UI 배치 |

파라미터 정의와 기본값은 Lua에 두고 프로젝트에는 선택한 값을 저장한다. effects 하위 폴더를 1초마다 추적해 Lua 파일을 자동 등록한다. 같은 파일명인 누락 항목과 신규 경로가 각각 하나일 때 ID·글로벌·기체 설정을 이전한다. 이름까지 바뀌거나 대응이 모호하면 기존 항목과 기체 Override를 삭제하고 새 파일을 기본값/빈 출력 매핑으로 등록한다. 삭제 파일의 복구용 설정은 남기지 않는다. 동일 Lua의 다중 인스턴스는 제외한다.

### 기체별 상속

우선순위는 Lua 기본값 → 프로젝트 글로벌 → 기체 Override다.

- 기체 파일에는 활성화, 개별 파라미터, 출력 이름별 목적지의 변경값만 저장한다.
- 누락은 상속, false는 명시적 비활성화, 빈 목적지 목록은 출력 없음이다. 출력 목록은 해당 이름 단위로 전체 교체한다.
- Reset to Global은 Override를 삭제한다. Specific 값이 글로벌 합성값과 같으면 Override를 제거한다. Global 값이 스크립트 기본값과 같으면 해당 저장 항목을 제거한다. Global에서는 Reset to Default를 제공한다.
- 어댑터의 정확한 기체 키로 매칭한다. 파일명·부분 이름·와일드카드로 추정하지 않는다. 미등록 기체는 Global, 중복 매칭은 오류로 처리한다.
- 다른 기체 편집은 현재 실행 프로필을 바꾸지 않는다. 첫 Override 저장 시 기체 파일을 생성한다.

### 논리 슬롯과 물리 모터

- Yoke, Stick, Throttle, Pedal, Wheel 각각 1–4번, 총 20개 슬롯이다.
- 한 슬롯에 여러 물리 모터를 연결하고, 같은 모터를 여러 슬롯에 연결할 수 있다. 프로젝트는 카테고리와 번호만 참조한다.
- 매핑은 PC 공통 로컬 설정이다. 기존 배정은 연결 해제 후에도 예약 상태로 유지한다.
- 신규 모터는 확인된 장치 정보와 명확한 좌우 이름을 이용해 카테고리를 판별하고 빈 슬롯부터 채운다. 좌우를 알면 Left부터 배정한다. 사용 중인 슬롯에 자동으로 합치지 않는다.
- 슬롯 부족·식별 불명은 Unassigned에 둔다. 사용자가 해제한 모터는 재실행해도 자동 배정하지 않는다. Unassigned는 출력 슬롯이 아니다.
- USB 연결 순서를 영구 식별자로 쓰지 않는다. 일련번호 없는 동일 모델 등 구분이 모호하면 수동 재배정을 요구한다.

## Lua 실행과 API

스크립트 하나는 상태를 가진 효과 하나다. 호스트가 인스턴스의 init(context), update(context, telemetry, parameters)를 호출한다. 상태는 self에 보관하며 destroy/exit 콜백 없이 호스트가 자원을 회수한다. 효과별 호출은 직렬화한다.

### 초기화

init은 파라미터·출력 목록을 등록하고 상태를 초기화한다. 비활성 효과도 UI 구성을 위해 초기화하되 출력은 허용하지 않는다. 등록 정의는 텔레메트리나 저장값에 따라 변하지 않는다. 등록이 끝나면 호스트가 글로벌·기체 값을 합성한다.

```lua
function effect:init(context)
    context:parameter {
        name = "Strength",
        type = "int",
        default = 20,
        min = 0,
        max = 255,
        step = 1
    }
    context:output("Center Gear", {"Stick:1"})
    context:output("Left Gear", {"Throttle:1"})
    context:output("Right Gear", {"Throttle:2"})
end
```

parameter는 type="boolean"/"int"/"float"/"haptic_power"와 default를 등록한다. 숫자는 min/max/step이 필요하다. haptic_power는 strength, pwm, on_ms, period_ms를 묶으며 강도 기본 범위 0–255, 기본 PWM 시간 20/80ms다. 복합 기본값과 선택적 min/max를 스크립트가 지정한다(CONTRACTS.md). output은 표시명·호출 키·저장 키를 겸하는 문자열 하나다. 효과 내 이름은 고유해야 한다. 출력마다 UI가 논리 목적지 선택 행을 만들며 여러 목적지 또는 빈 목록을 허용한다. 출력 이름 변경은 설정 키 변경이므로 연결을 추측 이전하지 않는다.

### 업데이트

새 텔레메트리가 들어오면 update를 호출한다. 마지막 호출 후 Maximum update period(기본 100 ms, 5–500 ms) 동안 새 데이터가 없으면 최신 스냅샷으로 다시 호출한다. 이는 목표 호출 간격이며 하드 실시간 보장이 아니다.

context는 API 메서드만 제공한다. init에서는 parameter/output/log, update에서는 log/vibrate/now/elapsed/sample_is_new/input_valid를 제공한다. now()/elapsed()의 단위는 ms다. telemetry는 수신 데이터와 샘플 메타데이터, parameters는 기본값→Global→기체 합성값을 담는 별도 인수다. context.telemetry/context.parameters 및 이전 필드 방식은 제공하지 않는다. 상태는 self에 유지한다.
재전달은 원래 샘플 시각이나 입력 만료를 갱신하지 않는다. 첫 입력 전·누락값은 무효로 표현하며 0으로 위장하지 않는다. 게임 자체의 중복 패킷·시뮬레이션 시각 정체도 구분한다. 게임 간 공통 텔레메트리로 변환하지 않는다. 게임별 원본 의미·단위를 유지하고 C++의 수신 시각·유효성·기체 식별은 실행 관리에만 사용한다. 필수 입력 미지원은 출력하지 않고 이유를 표시한다.

### 출력과 로그

```lua
context:log("Touchdown detected")
context:vibrate("Left Gear", constant(20), 300)
context:vibrate("Right Gear", pwm {
    strength = 20,
    on_ms = 20,
    period_ms = 80
}, 300)
context:vibrate("Left Gear", off())
```

- constant, pwm, off는 출력 사양 테이블을 만드는 헬퍼다. 시간 단위는 ms, HID 세기는 0–255다.
- vibrate는 자신의 등록된 출력에만 요청한다. 모터 직접 접근이나 다른 효과의 출력 취소는 허용하지 않는다.
- 출력 API는 콜백 동안 명령을 모으고 정상 종료 후 검증·일괄 반영한다. 로그는 효과 식별과 출력량 제한을 적용한다.
- Lua에 파일·프로세스·네트워크·HID 접근을 제공하지 않는다. 초기화와 업데이트 모두 실행량·메모리 제한을 적용한다.

## 출력 엔진 규칙

### 수명과 갱신

- 포트 상태는 On(활성 Constant/PWM 요청)과 Off다. PWM 내부 OFF 구간도 요청 자체는 On이다. 별도 대기 상태나 요청 큐는 없다.
- 새 vibrate는 기존 사양과 종료 기한을 교체하고 다음 HID 틱부터 반영한다. 종료 기한은 이번 update의 now_ms + duration_ms다. 이전 기한에 가산하거나 더 긴 기한을 강제하지 않는다.
- 요청이 없으면 기존 기한까지 유지한다. 기한 만료 또는 off는 해당 효과·포트만 해제한다. HID 전송 지연으로 기한을 다시 연장하지 않는다.
- 최소 진동 시간과 갱신 간격 때문에 생기는 끊김은 스크립트 작성자가 처리한다. 호스트는 요청 시간이나 세기를 자동 증대하지 않는다.

### 파형과 위상

- Constant는 지정 세기를 유지한다. PWM은 ON 시간과 전체 주기로 정의한다. OFF 시간은 주기에서 ON 시간을 뺀 값이다.
- PWM→PWM은 기준 시각을 유지하고 새 사양으로 즉시 계산한다. 갱신마다 처음부터 시작하지 않는다.
- PWM→Constant는 내부 OFF 구간을 기다리지 않고 전환한다. Constant/Off→PWM은 ON 구간부터 시작한다.
- 위상은 효과·포트별로 호스트가 관리한다. Lua에 restart 옵션을 제공하지 않는다.
- HID 틱 기본값은 20 ms이며 Settings → Set Timings에서 5–100 ms로 설정한다. PWM 시간은 가까운 틱으로 반올림하고 중간값은 올림한다. 유효 시간은 UI에 표시한다. ON=0은 무출력, ON=주기는 연속 출력이다. 반올림으로 0이 된 ON을 몰래 늘리지 않는다.

### 여러 요청의 우선순위

순간 출력값을 합산하거나 여러 파형을 겹치지 않는다. 각 최종 물리 모터에서 활성 후보 중 하나를 선택한 후 그 요청의 파형을 출력한다. 같은 효과·포트가 여러 매핑 경로로 도달하면 중복 후보로 세지 않는다.

| 후보 | 선택 기준 |
| --- | --- |
| Constant와 PWM | Constant 우선 |
| Constant끼리 | 높은 세기 |
| PWM끼리 | 높은 세기 → 짧은 주기 → 긴 ON 시간 |
| 완전 동률 | 현재 선택 유지. 최초 선택은 고정된 순서 |

Constant 우선은 세기와 별개의 규칙이다. Constant 1도 PWM 200보다 우선하며 다른 요청의 세기를 가져와 증폭하지 않는다. off·세기 0·유효 ON 시간 0은 경쟁 후보에서 제외한다.

선택된 PWM의 OFF 구간에는 다른 후보를 끼워 넣지 않는다. 선택되지 않은 요청도 원래 기한과 위상이 진행한다. 우선 요청 종료 후 남은 후보를 다시 선택하며 재시작하지 않는다. 이 순서는 제품의 우선순위이며 서로 다른 하드웨어의 체감 세기 순서를 보장하지 않는다.

### 종료와 모니터

- 초기 텔레메트리 만료는 500 ms다. 연결 해제, 만료, 기체 변경, Stop, 효과 비활성화·오류·재적용 시 관련 출력을 정리한다.
- 예약 기한과 런타임 정상 동작 감시는 별개다. 실행기 응답 중단도 출력을 해제해야 하며 실행기 heartbeat 제한은 250ms다. 정상 효과는 같은 출력을 계속 재요청할 필요가 없다.
- UI·Lua 실행과 HID 루프를 분리한다. 스크립트 실행 지연이 출력 만료나 Stop을 막지 않게 한다.
- Motor Monitor는 연결된 모터만 표시한다. Motor 이름 열은 넓게, HID 열은 52px로 표시하고 Status/Connected 열은 제거했다. HID는 마지막 전송에 성공한 0–255 강도이며 20ms 간격 UI 스냅샷이다(장치 입력 축이나 실측 진동이 아님). 연결 해제 시 행을 숨기되 매핑은 예약한다. 오류는 Debug Output에 기록한다.
- 모터 테스트는 명시적 Start와 제한 시간을 요구한다. 테스트 모터의 게임 출력을 잠시 제외하고 같은 엔진으로 구동한다. 전역 Stop은 테스트도 중단한다.

## 편집과 적용

파라미터·활성화·매핑 수정은 즉시 실행에 반영하며 저장은 별도다. 파라미터 변경은 기존 Lua self와 PWM 위상을 유지한다. 스크립트는 저장 시 해당 효과만 검증·교체한다. 오류는 로그에 표시하며 해당 효과의 기존 유효 버전은 유지하고 다른 효과의 수정은 계속 반영한다. 저장하지 않은 코드 초안은 실행하지 않는다. Ctrl+S는 현재 스크립트와 그 효과의 글로벌/기체 설정만 저장하고, Ctrl+Shift+S는 전체 저장한다. Close File은 편집 파일만 닫으며 효과를 비활성화하지 않는다.

프로젝트 전환·재초기화 시 호스트가 출력·예약·상태를 정리하고 이전 인스턴스의 지연 명령을 폐기한다. 파일은 임시 파일을 통한 교체로 저장한다. 사라진 파라미터·출력 이름의 설정은 조용히 삭제하지 않고 미해결 항목으로 표시한다. 지원하지 않는 상위 파일 형식 버전은 거부한다.

## 초기 효과와 구현 경계

Ground Roll은 이동·접촉 조건을 만족하면 지정 세기를 요청한다. Touchdown은 바퀴별 접촉·압축 상승·재무장 상태를 Lua에서 관리하고, 첫 상승 구간의 시간 × 배수에 최소/최대 제한을 적용한다. 이후 더 높은 피크는 무시한다. 측정 종료 샘플과 중복 데이터 처리는 CONTRACTS.md와 실제 Lua 파일에 정의한다. 최소 시간 재요청 방식의 구현·디버깅도 Lua 효과의 책임이다.

구현 책임은 UI / 프로젝트·프로필 / Lua 런타임 / 게임 어댑터 / 출력 엔진·HID로 나눈다. 상속 프레임워크나 별도 서비스는 만들지 않는다.

## 구현 및 실행 (2026-11-09)

- 프로젝트가 선택한 MSFS 연결은 2초마다 재시도한다. Settings 없이 2024는 공식 `bin/simconnect/MSFS2024/SimConnect.dll`, 2020은 `bin/simconnect/MSFS2020/SimConnect.dll`을 로드한다. 게임 내부 DLL이나 PATH를 검색하지 않는다.
- HID는 1초마다 검색하고 연결 유지 중인 장치 핸들은 재사용한다. 지원 목록은 아래 WINCTRL HID 표를 따른다. Aurora에 정의된 장치별 프로토콜만 사용하며, 미등록 PID에는 출력하지 않는다. 일련번호 없는/중복 장치는 세션 내 수동 매핑이 필요하다.
- 마지막 프로젝트를 시작 시 복원한다. 해당 파일을 못 찾으면 빈 화면이며 다른 최근 프로젝트를 대신 열지 않는다. File → Open Recent에서 최근 10개를 열 수 있다.
- Explorer는 파일 트리 하나다. 등록된 Lua 옆 체크박스는 Parameters에서 선택한 프로필의 enabled를 제어한다. 폴더를 접고 펼칠 수 있고, 파일 더블클릭으로 탭을 연다. 탭은 이동·닫기가 가능하며 미저장 파일/설정은 *로 표시한다.
- Parameters 상단에서 Global 또는 Specific Aircraft: TITLE을 선택한다. 실제 실행 기체는 수신된 TITLE로 결정된다. Global의 Reset All to Default는 선택 효과의 글로벌 활성화/파라미터/출력 설정을 지워 기본값(true/등록 파라미터·출력 기본값)으로 되돌린다. Specific의 Reset All to Global은 선택 효과의 해당 기체 Override 전체를 제거한다. 스크립트는 기체와 무관한 공통 파일이다.
- New Project는 Project Directory, Target Game(MSFS 2024 / MSFS 2020 / X-Plane 12 / War Thunder), Generate default Lua scripts를 받는다. 체크 시 선택한 게임의 예제를 복사하며, 해제 시 기본 스크립트 없이 프로젝트를 만든다. 없는 절대 경로는 생성한다. 기존 빈 폴더는 확인 없이 사용하고, 내용이 있는 폴더만 확인 후 프로젝트 파일과 이름이 겹치는 기본 Lua를 교체하고 나머지 파일은 보존한다. 보존된 effects 내 Lua도 자동 발견된다. Left Gear→Throttle:1, Right Gear→Throttle:2, Center Gear→Stick:1을 Lua 등록에서 기본값으로 선언한다.

### 빌드와 배포

MSVC Build Tools 2026 (19.51), Windows SDK 10.0.26100.0, 포함 CMake를 사용한다. `.deps`는 Git에서 제외한다.

```powershell
.\scripts\bootstrap.ps1
.\scripts\build.ps1 -Deploy
```

`build.ps1`은 `build\Release`에 Qt DLL·플러그인, MSVC CRT, effects, 라이선스·소스와 공식 SimConnect.dll을 배치한다. `-Deploy`는 같은 구성을 `dist\StellaVibra`에도 만든다. 공식 클라이언트가 `.deps/msfs2024/client`에 없으면 배포 단계에서 명시적으로 실패한다. 현재 두 폴더는 로컬 실행·검증용이며, Microsoft DLL의 공개 재배포 권리 확인은 기존 SIMCONNECT-LICENSING.md 검토대로 남아 있다. 앱 소스의 MIT 라이선스와는 별개다.

### 사용자 수동 검증

1. `build\Release\StellaVibra.exe` 실행 → File/New Project → 두 스크립트를 열고 서로 다른 초안을 만든 뒤 하나만 저장, 다른 초안 유지와 Save All 확인. 재시작 복원과 없는 마지막 프로젝트의 빈 화면 확인.
2. effects 아래 폴더로 파일 이동 → 설정 유지 확인. 파일명 변경·삭제 → `.sv`와 기체 파일에서 이전 효과 설정 삭제 확인.
3. 숫자 슬라이더/입력, PWM 체크와 ON/Period, 기체 Override와 Reset All to Global 확인. 저장하지 않은 코드가 실행되지 않고 저장 시 해당 효과만 갱신되는지 확인.
4. MSFS 시작/종료와 지원 HID 연결/분리 → 상태 자동 갱신 확인. 실제 기체 TITLE, pause, 입력 만료를 확인.
5. 매핑과 낮은 세기로 Ground Roll/Touchdown, Stop All/Resume All 확인. Touchdown 첫 상승·바퀴별 재무장과 파라미터 조절 중 상태 유지 확인.

개발 측 검증은 컴파일·링크·정적 점검이다. GUI, 실제 DLL/게임 연결, 기체별 기어 신호, HID 좌우·진동·타이밍은 사용자 검증 전이다. 강제 프로세스 종료나 드라이버 장애에서 물리 모터 정지는 검증하지 않았다.

## WINCTRL HID 지원 (2026-11-10)

모두 VID `4098`. 기존 스틱 미감지 원인은 `B970`만 허용했던 필터다. 기체 파라미터나 Explorer 숨김과 무관하다. 이번 지원은 Aurora의 `src/hid/protocol.rs`, `worker.rs`, `win32.rs`를 대조해 구현했다. 실제 연결 장치의 PID/descriptor/진동은 실행 검증하지 않았다.

| 모델 | PID | 독립 출력 |
| --- | --- | --- |
| URSA MINOR Airbus L / R | BC27 / BC28 | 스틱별 1 |
| URSA MINOR Combat(Fighter) L / R | BC29 / BC2A | 스틱별 1 |
| URSA MINOR Space L / R | BC2B / BC2C | 스틱별 1 |
| URSA MINOR Throttle / Throttle R | B920 / B930 | Left / Right |
| Combat Throttle Metal.EX | B970 | Left=A / Right=B |
| Orion Joystick Base II | BEA8 | 1 |
| Orion Throttle Base II | BD64 | Combined 1 (동일 강도로 두 명령 전송) |

스틱은 Stick:1–4, 스로틀은 Throttle:1–4 빈 슬롯에 자동 배정한다. 예제의 기본 목적지는 여전히 Throttle:1/2이므로 스틱에 출력하려면 효과 출력 목적지에 배정된 Stick 슬롯을 추가한다. 한 모터에는 기존 우선순위로 요청 하나만 선택하며 파형을 합산하지 않는다. 정상 descriptor 없이 보고서 형식을 추정해 전송하지 않는다. 장치 발견/분리·알 수 없는 WINCTRL PID·descriptor 및 쓰기 접근 오류는 Debug Output에서 확인한다.

빌드는 기본적으로 전체 출력을 `build/build.log`에 보관하고 성공 시 결과·로그 경로 두 줄만 표시한다. 실패 시 오류 최대 12줄(오류 추출이 없으면 로그 마지막 12줄)과 경로를 표시하며 실패 코드로 종료한다. `.\scripts\build.ps1 -Deploy -Detailed`는 상세 출력을 콘솔에 표시한다. MSBuild도 `/verbosity:minimal /nologo`를 사용한다.

## 파일 탭과 원시 JSON 편집

Effect Control은 파일별 Editor 문서를 탭으로 연다. 탭 전환 시 커서·스크롤·실행 취소·미저장 초안을 보존하고, 전체 상대 경로를 툴팁으로 표시한다. Lua는 Parameters와 함께, 프로젝트/aircraft `.sv`는 파라미터 없이 JSON 소스로 표시한다. 상단에는 자동 생성 파일이므로 수동 수정을 권장하지 않는다는 안내를 둔다.

JSON 저장은 문법·형식 버전·프로젝트 및 모든 기체 파일의 스키마·중복 기체 키/효과 ID·경로를 검증한 후 수행하고, 저장된 설정을 실행기에 다시 전달한다. Lua 설정 검증/오류 격리는 기존 실행 계약을 따른다. Ctrl+S는 현재 파일, Save All은 모든 초안이다. 열린 JSON이 디스크에서 바뀌면 오래된 초안으로 덮어쓰지 않고 다시 열도록 안내한다.

자동 생성 설정과 raw 편집의 덮어쓰기를 막기 위해, 파라미터 초안이 있으면 먼저 저장/취소한 뒤 JSON을 편집한다. JSON 초안이 있으면 파라미터·활성화 변경과 effects 구조 자동 반영을 잠시 보류한다. 디스크 트리 자체는 계속 갱신된다. JSON 저장/취소 후 자동 추적을 재개한다. JSON 문서를 보기만 하는 것은 추적을 막지 않는다. 직접 수정은 현재 프로젝트와 그 프로젝트의 기체 파일에 한정한다.

Motor Monitor는 Yoke → Stick → Throttle → Pedal(러더) → Wheel, 각 1–4 순서다. 다중 매핑 모터는 가장 앞선 슬롯을 정렬 기준으로 쓰며 같은 슬롯은 장비 이름 오름차순, 동일 이름은 장치 ID순이다. 미배정은 마지막이다. Slots 셀을 우클릭/더블클릭하면 매핑, HID 셀은 모터 테스트 창을 연다. 테스트 창을 열기만 하면 구동하지 않는다. 하단은 Stop All/Resume All만 유지한다.

## 간결한 파라미터와 최소 저장

Power는 강도 슬라이더/숫자, PWM, ON/Period를 한 줄로 표시한다. PWM을 켜면 시간 입력이 같은 행에 나타나고 슬라이더가 남은 폭을 사용한다. ON/Period 단위는 ms다. 각 값 바로 옆의 Default/Global/Specific은 실제 값의 출처다. 체크박스처럼 작은 컨트롤 뒤에 고정 너비 공백을 두지 않는다.

Global에서는 Reset to Default, Specific에서는 Reset to Global로 상위 값을 상속한다. 상위 값과 같아진 enabled/파라미터/출력 항목은 메모리에서 제거하고 저장 시 `.sv`에서도 생략한다. enabled 기본값은 true, 출력 목적지 기본값은 Lua 등록값이며 생략 시 빈 목록이다. 숫자/boolean/haptic_power의 기본값은 Lua 등록값이다. haptic_power는 전체 객체가 상속 단위이므로 숨겨진 PWM 시간도 비교에 포함한다. 빈 그룹과 빈 효과 Override도 제거하며 알 수 없는 설정 키는 보존한다. 기존 중복값은 의미를 유지한 채 일반 Save/Save All에서 정리된다.

수정 표시는 변경 이력의 존재가 아니라 저장 상태와의 차이를 뜻한다. 값을 1→2→1로 돌려 모든 설정이 저장 당시와 같아지거나 Lua/JSON을 실행 취소해 저장 텍스트로 돌아오면 별표와 저장 질문이 해제된다. 다른 파일/프로필에 변경이 남으면 해당 파일 또는 앱의 수정 표시는 유지한다.

Outputs는 출력 이름을 가로 제목으로 하는 표다. 아래 선택 셀에는 여러 목적지를 한 줄씩 표시하고, 셀에서 목적지를 선택한다. 출력별 출처/Reset도 같은 셀 안에 표시한다. 출력이 많으면 가로 스크롤한다.

## Lua 파워 직접 전달과 편집 보조

`context:vibrate(name, parameters.Power, duration_ms)`로 상속이 합성된 haptic_power 객체를 직접 전달한다. mode가 없으면 pwm boolean으로 Constant/PWM을 선택하며 기존 강도·시간 검증, 위상·기한 규칙을 그대로 적용한다. 객체를 변경하지 않는다. 명시적인 mode가 있으면 우선하고 기존 constant/pwm/off 헬퍼도 유지하므로 시간에 따른 강도 계산도 가능하다. 신규 프로젝트용 예제는 직접 전달을 사용한다. 기존 프로젝트의 Lua 파일은 자동으로 덮어쓰지 않는다.

File과 View 사이 Edit 메뉴는 활성 Lua/JSON 문서의 Undo(Ctrl+Z), Redo(Ctrl+Y 또는 Ctrl+Shift+Z)를 제공한다. 파라미터 변경 이력을 위한 Undo 스택은 추가하지 않는다.

Lua 편집기는 init/update의 context: 메서드, telemetry.의 지원 필드, parameters.의 init 등록 이름을 제안한다. 파라미터 공백은 _로 바꾸고 Lua 식별자/예약어/충돌을 검증한다. 표시명과 .sv 저장 키는 원래 이름이다. parameters.Power.에서는 haptic_power 필드를 제안한다. 자동완성은 초안의 문자 그대로 쓴 name/type 등록을 읽으며 코드를 실행하지 않는다. 임의 별칭/계산된 이름/LSP 추론은 제공하지 않는다. Enter/Tab 선택, Escape 취소, Ctrl+Space 재요청을 지원한다.

## 2026-11-11 편집·진단 확장

이 버전은 개발 중 API 변경으로 이전 Lua 호출 방식과 호환하지 않는다. 기존 프로젝트 파일은 덮어쓰지 않는다. 새 프로젝트를 생성하거나 기존 Lua를 init(context), update(context, telemetry, parameters)로 수정한다. parameters["Minimum speed knots"]는 parameters.Minimum_speed_knots로, context.now_ms는 context:now()로 변경한다.

Explorer는 아이콘 앞의 체크박스로 effects 하위 폴더와 개별 Lua를 제어한다. 최상위 effects 폴더에는 체크박스가 없다. 별도 On 열은 사용하지 않는다. 일부만 켜진 폴더는 부분 체크다. 현재 편집 중인 Global/Specific 프로필에 적용하며 기체/.sv 파일은 체크박스가 없다. Enabled/Disabled는 Parameters 상단의 별도 버튼이며 Lua 파라미터가 아닌 프로젝트 활성화 상태다. 파라미터 앞의 설명문과 enabled 행을 제거했고 Power 전용 최대 너비 제한도 제거했다.

Telemetry 도킹 창은 Motor Monitor 아래에 있다. 실제 수신한 지원 필드의 값·단위·원 SimVar 이름(툴팁)을 표시하며 미수신은 Unavailable, 연결 종료/일시정지/만료는 Disconnected/Paused/Stale로 구분한다. 100ms마다 화면만 갱신하며 수신 시각과 유효기간은 연장하지 않는다. MSFS 전체 SimVar 사전이나 기체 전용 L-var 탐색기는 아니다. View 마지막 Reset Default Layout은 숨긴 창까지 기본 배치로 복원한다.

Settings → Set Timings는 HID tick(5–100ms, 기본20)과 Maximum update period(5–500ms, 기본100)를 설정한다. Reset to Defaults는 두 값을 기본값으로 돌리며 OK에서만 적용·저장하고 Cancel은 취소한다. 실행 폴더의 StellaVibra.pref에 tick_ms/maximum_update_period_ms로 저장한다. HID tick 변경 시 테스트/활성 요청을 제거하고 Lua 상태를 재초기화한다. 최대 업데이트 간격만 변경하면 상태를 유지한다. 새 텔레메트리는 간격과 무관하게 즉시 처리하고, 새 입력이 없을 때만 이 간격으로 기존 스냅샷을 재전달한다. 무효·만료 입력은 효과를 실행하지 않으며 재전달로 입력 수신 시각·유효기간·출력 기한을 갱신하지 않는다. 게임 구독 주기와 Telemetry 뷰의 100ms 갱신 주기는 변경하지 않는다. OS/USB의 실제 타이밍 정밀도를 보장하지 않는다. Help → About은 MIT/Qt LGPL/Aurora 고지와 타사 라이선스 안내를 표시한다.

MSFS 기본 효과는 ground(Ground Roll, Touchdown, Ground Thumps), aerodynamics(Stall, Overspeed, Spoilers, Flaps, Bank), engines(Startup, Shutdown, Thrust) 11개다. Aurora의 표준 입력 조건을 참조하되 게임 판단은 모두 Lua에 있다. Ground Thumps는 슬랩 간격/속도 기반 모델이며 실제 노면 충격 센서가 아니다. Flaps는 실제 플랩/슬랫 값 변화+짧은 유지 시간, Spoilers는 좌우 전개/대칭, Bank는 비행 중 뱅크각을 사용한다. 기체 전용 L-var 보정과 Aurora의 파형 혼합/레거시 강도 곡선은 가져오지 않는다. 엔진 Startup은 starter/combustion 상승, Shutdown은 combustion 하강 후 감쇠, Thrust는 제트/프로펠러 추력을 사용한다. 최초 샘플은 엔진 edge를 만들지 않는다. Full scale pounds와 Propeller thrust는 기체에 맞춰 조정한다. 표준 SimVar를 갱신하지 않는 애드온은 별도 지원되지 않는다.

새 프로젝트의 Airframe 출력은 Stick:1, Engine 1–4는 Throttle:1–4, Left/Right Gear는 Throttle:1/2에 연결하며 Center Gear는 Stick:1이다. 이 매핑은 C++ 이름 규칙이 아니라 각 Lua의 output 등록에서 선언한다. 원하는 장치/슬롯으로 변경해야 한다. 여러 효과의 요청은 기존 우선순위로 하나만 선택한다.

공식 입력 근거: [MSFS 2024 SimVars](https://docs.flightsimulator.com/msfs2024/retail/programming-apis/simvars/simulation-variables/), [Engine variables](https://docs.flightsimulator.com/msfs2024/retail/programming-apis/simvars/aircraft-simvars/aircraft-engine-variables/). 실제 구독명/단위는 src/telemetry.h의 공통 목록이며 SimConnect 등록·디코딩·자동완성·Telemetry 뷰가 공유한다.

시작 배치 복원은 현재 layout_version=4일 때만 한 번 수행한다. 이전 버전은 새 기본 배치를 사용하므로 설정 파일을 삭제할 필요가 없다. `StellaVibra.exe --ui-check`는 하드웨어/게임 스레드·프로젝트 자동 복원·설정 저장 없이 창 표시와 기본 배치 복원을 확인하고 2초 후 종료하는 진단 모드다.

## 기본 배치 및 효과 트리

기본 좌우 폭은 Project Explorer : Effect Control : Motor Monitor/Telemetry = 1:4:1, 본문 : Debug Output 높이는 3:1, Parameters : Script Editor는 1:1을 목표로 한다. 창 크기를 바꿀 때 마지막 사용자 조정 비율을 유지한다. 각 도크와 Parameters/Script의 최소 크기 제약을 0에 가깝게 풀었으며 작은 영역에서는 내용이 잘릴 수 있다. 제목줄·분할선·픽셀 반올림 때문에 비율은 근사값이다. 분리한 부동 창은 비율 적용 대상에서 제외한다. View → Reset Default Layout에서도 같은 비율을 적용한다. 배치 버전4 이전의 저장 배치는 새 기본 배치로 교체하며 이후 사용자 조정은 저장한다. 기존 저장 배치는 유지하므로 새 3:1 기본값을 적용하려면 Reset Default Layout을 사용한다.

체크박스는 각 파일/하위 폴더 아이콘 앞에 배치해 같은 깊이에서 정렬한다. 최상위 effects 폴더의 체크박스는 생략한다. 폴더 부분 체크/일괄 토글과 Space 조작을 유지하고 .sv/aircraft에는 표시하지 않는다. 프로젝트를 열면 effects의 모든 하위 폴더를 펼치며, 비동기 디렉토리 로딩도 처리한다. 열린 뒤 사용자가 접은 폴더를 주기적으로 다시 펼치지는 않는다.

현재 Telemetry/자동완성 목록은 공식 SimVar 전체가 아닌 앱의 구독 목록(숫자/boolean59개 + TITLE 및 호스트 메타데이터)이다. SimConnect는 AddToDataDefinition으로 지정한 이름·단위를 RequestDataOnSimObject로 요청하는 방식이므로 게임 연결만으로 목록이 늘어나지 않는다. 추가 항목은 공식 정의에 따라 구독 목록과 Lua 계약에 등록해야 한다. 기체별 값의 의미/갱신 여부가 다르며 0이나 정체를 미지원 증거로 사용하지 않는다. 따라서 연결 시 자동완성 키를 임의로 추가/삭제하지 않는다. EnumerateInputEvents는 현재 기체의 InputEvent 목록으로, 전체 SimVar의 지원 여부 목록이 아니다.

근거: [AddToDataDefinition](https://docs.flightsimulator.com/msfs2024/retail/programming-apis/simconnect/api-reference/events-and-data/simconnect_addtodatadefinition/), [RequestDataOnSimObject](https://docs.flightsimulator.com/msfs2024/flighting/programming-apis/simconnect/api-reference/events-and-data/simconnect_requestdataonsimobject/), [EnumerateInputEvents](https://docs.flightsimulator.com/msfs2024/retail/programming-apis/simconnect/api-reference/inputevents/simconnect_enumerateinputevents/). SDK Core1.7.3 헤더의 열거 함수도 대조했으며, 전체 표준 SimVar 지원 목록을 현재 기체에서 반환하는 API는 확인되지 않았다. 실제 게임 연결/변수 탐색은 수행하지 않았다.

## 텔레메트리 확장 검토

MSFS 기존 48개+TITLE에 아래 10개 입력과 ON ANY RUNWAY를 추가하여 59개+TITLE을 구독한다. 이것은 각 애드온이 표준 SimVar를 정확히 제공한다는 보장이 아니다. 기존 효과의 판단 알고리즘은 유지하며 새 입력은 Lua 작성자가 사용할 수 있다.

| 우선순위 | 후보 | 활용 및 한계 |
| --- | --- | --- |
| 1 | SURFACE TYPE | Ground Roll/Thumps를 잔디·포장·물 등으로 구분. 실제 요철 크기나 슬랩 위치를 측정하는 값은 아니다. |
| 2 | G FORCE, ACCELERATION BODY X/Y/Z | 하중과 기체 흔들림을 이용한 반응형 효과. 축/단위·중력/정상기동 성분·노이즈를 고려해 Lua에서 필터링한다. 난기류만을 분리하는 센서는 아니다. |
| 3 | INCIDENCE ALPHA, STALL ALPHA | 현재 stall_warning ON/OFF 대신 실속 임박 정도에 따른 점진적 버핏을 설계. 기체별 검증과 각도 단위 일치가 필요하다. |
| 4 | GEAR TOTAL PCT EXTENDED, BRAKE LEFT/RIGHT POSITION | 기어 작동/전개 및 제동 효과. 접지·속도 조건과 함께 사용하며 브레이크 위치를 실제 감속/마찰력으로 취급하지 않는다. |

Ground Thumps는 우선 SURFACE TYPE으로 개선할 가치가 크다. G FORCE/가속도는 새 기체 반응 효과로 분리하고, 기존 Touchdown의 첫 압축 상승 판단을 대체하지 않는다. 이미 구독 중인 엔진 RPM%/N1/N2는 기본 예제에서 직접 사용하지 않으므로 엔진 스풀 효과를 확장할 때 우선 재사용할 수 있다.

근거: [Miscellaneous variables](https://docs.flightsimulator.com/msfs2024/retail/programming-apis/simvars/aircraft-simvars/aircraft-miscellaneous-variables/), [Flight model variables](https://docs.flightsimulator.com/msfs2024/html/6_Programming_APIs/SimVars/Aircraft_SimVars/Aircraft_FlightModel_Variables.htm), [Brake and landing gear variables](https://docs.flightsimulator.com/msfs2024/flighting/programming-apis/simvars/aircraft-simvars/aircraft-brake-and-landing-gear-variables/).

## 게임별 연결과 기본 Lua

프로젝트 `game`은 `MSFS2024`, `MSFS2020`, `XPlane12`, `WarThunder` 중 하나다. 프로젝트가 없으면 게임 연결을 시작하지 않는다. 프로젝트 종료·게임 변경 시 기존 연결과 출력 요청을 해제한다. 원격 게임 연결은 이번 범위에 포함하지 않는다.

| 게임 | 연결·Lua 입력 | 기본 스크립트 |
| --- | --- | --- |
| MSFS 2024 / 2020 | 각 버전의 공식 SimConnect DLL. `telemetry.ground_speed_knots` 등 기존 MSFS 계약 + 새 11개 필드 | 각각 11개: ground 3 / aerodynamics 5 / engines 3 |
| X-Plane 12 | `127.0.0.1:49000` UDP RREF, Aurora의 86개 dataref. `telemetry["sim/flightmodel/position/groundspeed"]`처럼 원본 경로·숫자 값 사용 | 11개. 지상 속도 m/s, 타이어 수직 압축 m. 엔진 진동은 추력 대신 RPM 기준 |
| War Thunder | `127.0.0.1:8111/state`, `/indicators`, 최대 20 Hz. 응답 JSON 전체를 `telemetry.state`, `telemetry.indicators`에 보존 | 8개: AoA 기반 Stall cue / 수동 Overspeed / Flaps / Gear / Firing / Startup / Shutdown / Power |

War Thunder의 `telemetry.state["IAS, km/h"]` 등 공백·단위가 들어간 키는 대괄호로 접근한다. 원본 `valid`, `type`, `weapon1` 같은 식별자는 점 접근도 가능하다. 자동완성은 프로젝트 게임별로 구분한다. X-Plane은 `telemetry["`, War Thunder는 `telemetry.state["`/`telemetry.indicators["`에서 후보를 표시한다. War Thunder의 실제 수신 키도 후보·Telemetry 뷰에 추가하며, 알려진 기본 키는 접속 전에도 유지한다. 값 0을 미지원 근거로 삼지 않는다.

X-Plane은 빠른 필드 20 Hz, 기체 정보 1 Hz로 구독한다. 타이어 압축에서 접촉과 첫 압축 상승을 판단하는 것은 Lua의 규칙이며 기체별 확인이 필요하다. RPM 효과는 실제 추력 측정이 아니다. War Thunder는 확인된 입력에 개별 바퀴 접지·압축이 없어 Ground Roll/Touchdown 예제를 제공하지 않는다. AoA·과속 한계는 기체별로 조정해야 하며 무장 번호도 기체마다 다를 수 있다. 첫 엔진 샘플만으로 시동·정지 edge를 만들지 않는다.

MSFS 2020 클라이언트도 2024에 연결될 수 있으므로 선택한 로컬 프로세스가 있는지 확인한다. 두 MSFS 버전이 동시에 실행 중이면 기본 파이프의 모호성을 피하기 위해 연결을 보류하고 로그에 다른 버전 종료를 안내한다. 다른 게임으로 자동 대체 연결하지 않는다.

### 전체 텔레메트리 일괄 구독 판단

모든 게임의 모든 변수를 고빈도로 수신하지 않는다. SimConnect는 [정의당 1,000개 제한](https://docs.flightsimulator.com/html/Programming_Tools/SimConnect/API_Reference/Events_And_Data/SimConnect_AddToDataDefinition.htm)이 있고, 문자열·구조체·배열 인덱스·애드온 전용 값은 별도 타입·단위·범위 계약이 필요하다. 전체 SimVar 목록이 각 기체에서 모두 유효하다는 보장도 없다. 현재는 검증된 59개 숫자/bool+TITLE 카탈로그를 사용한다.

X-Plane 12.1.1 이상의 [공식 Web API](https://developer.x-plane.com/article/x-plane-web-api/)는 dataref 목록 조회를 제공한다. 다만 목록 열거와 전체 값의 고빈도 스트리밍은 별개이며, 모든 배열·플러그인 필드를 무제한 구독하면 대역폭·처리량·재연결 부담이 커진다는 것이 이번 설계 판단이다. 이번 구현은 Aurora와 같은 RREF 86개 카탈로그로 제한한다. Web API 탐색이나 임의 구독 UI는 추가하지 않았다.

War Thunder는 이미 요청한 두 응답 안의 모든 키를 통과시킨다. 이것이 게임의 전체 API를 뜻하지는 않는다. 지도·HUD·미션 엔드포인트나 확인되지 않은 기체 전용 값까지 추가 폴링하지 않는다. C++에서 게임별 값을 공통 speed/gear/engine 필드로 변환하지 않으며, 판단·단위 변환·상태·재무장은 각 Lua가 담당한다.

## 실행 폴더별 설정과 초기화

앱 설정은 `StellaVibra.exe` 옆 `StellaVibra.pref`에 UTF-8 JSON으로 저장한다. 창 위치·크기, 도킹 뷰의 위치·표시 상태, 마지막/최근 프로젝트 경로, 물리 모터 매핑과 HID 틱·최대 업데이트 간격 설정을 포함한다. 프로젝트 내부의 파라미터·기체 Override·Lua는 해당 프로젝트 파일에 계속 저장한다.

- 바로가기의 시작 위치나 현재 작업 디렉토리와 관계없이 실제 EXE 디렉토리를 기준으로 한다. 여러 폴더에 배치하면 폴더마다 독립 설정을 사용한다. 폴더 전체를 복사하면 포함된 pref 설정도 복제되므로 새 기본값을 원하면 복사본의 pref를 삭제한다.
- 설정 파일이 없으면 기본값으로 시작하며 첫 설정 저장 시 파일을 생성한다. 앱을 종료한 뒤 pref만 삭제하면 앱 옵션을 초기화할 수 있다. 실행 폴더 전체를 삭제해도 앱 옵션이 남지 않는다. 외부 디렉토리에 저장한 프로젝트까지 삭제되는 것은 아니다.
- 기존 위치는 `%LOCALAPPDATA%\StellaVibra\StellaVibra\local.json`이었다. 새 버전은 그 파일을 읽거나 자동으로 이전하지 않는다. 기존 파일은 그대로 두며, 필요 없으면 사용자가 삭제할 수 있다. 설정을 유지하려면 앱 종료 후 그 파일을 원하는 실행 폴더에 `StellaVibra.pref`라는 이름으로 복사한다.
- 실행 폴더에는 쓰기 권한이 필요하다. 저장 실패 시 오류를 표시하고 AppData나 레지스트리에 우회 저장하지 않는다. 빌드·배포 작업은 기존 pref를 덮어쓰지 않는다. 프로젝트 경로는 절대 경로이므로 폴더 복사/이동 후 프로젝트 위치가 달라졌다면 다시 Open해야 한다.

## Lua 출력 기본 채널

`init`에서 `context:output(name, default_slots)`로 출력과 기본 논리 슬롯 목록을 함께 등록한다. 예:

```lua
context:output("Center Gear", {"Stick:1"})
context:output("Left Gear", {"Throttle:1"})
context:output("Right Gear", {"Throttle:2"})
context:output("Airframe", {"Stick:1", "Throttle:1"})
context:output("Optional") -- No default destinations.
```

두 번째 인수 생략 또는 `{}`는 미배정이다. Yoke/Stick/Throttle/Pedal/Wheel의 1–4번 슬롯만 허용하며 배열에 중복·잘못된 이름·빈 인덱스가 있으면 등록 오류다. 논리 슬롯의 실제 장치 매핑은 실행 폴더 설정을 따른다.

출력도 Script Default → Global → Specific 순서로 합성한다. 저장된 빈 배열은 기본 채널로 돌아가는 뜻이 아니라 명시적 미배정이다. 기본값과 같은 Global 설정은 `.sv`에서 생략한다. 새 프로젝트 생성 시 C++이 출력 이름으로 채널을 추측하거나 기본값을 Global로 복사하지 않는다. 기본 제공 Lua의 모든 output 등록은 반복문 없이 명시적으로 나열하며 update의 처리 반복문은 유지한다. 기존 사용자 프로젝트의 Lua는 자동으로 덮어쓰지 않는다.

New Project의 Browse는 폴더 트리와 경로 입력을 제공한다. 폴더를 선택하거나 현재 폴더 기준 새 이름/하위 경로 또는 절대 경로를 입력한 뒤 Select Folder를 누르면 없는 경로를 부모 폴더까지 자동 생성한다. 생성 확인창은 표시하지 않고 실패 시 오류만 표시한다. 그 뒤 New Project를 Cancel해도 이미 생성한 빈 폴더는 남는다. 빈 폴더는 프로젝트 생성 시 추가 확인이 없으며 숨김/시스템 항목 또는 하위 폴더가 하나라도 있으면 기존 내용 보호 확인을 유지한다.

## 비정상 종료 진단

일반 실행 시 배포 루트의 `diagnostics/<UTC시각>-<PID>.log`에 Qt 진단·Debug Output·SimConnect 연결 시도/결과·첫 텔레메트리·첫 Lua update·첫 HID 출력 시점을 기록한다. 각 로그는2MiB 초과 시 `.log.previous` 한 개를 유지하며 현재 로그를 새로 쓴다. 실패를 숨기려고 AppData에 저장하지 않는다. `--ui-check`에서는 파일 진단을 초기화하지 않는다.

처리되지 않은 Windows 예외가 발생하면 같은 세션명의 `.dmp` 미니덤프 생성을 시도한다. 이는 손상된 프로세스 내 최선의 시도이며 모든 종료·강제 종료·fail-fast·쓰기 불가에서 덤프를 보장하지 않는다. [Microsoft MiniDumpWriteDump 안내](https://learn.microsoft.com/en-us/windows/win32/api/minidumpapiset/nf-minidumpapiset-minidumpwritedump)에 따라 프로세스 내부 수집의 한계를 명시한다. 정상 동작을 복구하거나 예외를 무시하지 않는다. 파일은 자동 전송하지 않는다. 분석용 bin/StellaVibra.pdb를 Release/dist에 동봉한다. 덤프 발생 당시 bin/StellaVibra.exe 및 bin/StellaVibra.pdb와 해당 log/dmp를 함께 보존해야 한다.

MSFS 연결을 닫을 때 연결 핸들은 해제하지만 SDK 모듈은 [GetModuleHandleEx PIN](https://learn.microsoft.com/en-us/windows/win32/api/libloaderapi/nf-libloaderapi-getmodulehandleexa)으로 프로세스 종료까지 유지한다. 연결 실패/재시도 중 DLL 내부 비동기 작업 코드가 사라지는 위험을 줄이기 위한 수명 관리다. 이 변경만으로 보고된 크래시의 원인이 확정되거나 재현 검증되었다고 보지 않는다.

## 기본 효과 조정과 채널 수 (2026-11-12)

Settings → Set Channels에서 Yoke/Stick/Throttle/Rudder의 채널 수를 각각0..4로 선택한다(기본4). Rudder의 내부 슬롯 이름은 기존 Pedal이다. Lua가 요청한 기본 출력은0개인 카테고리에서 미배정되며, 나머지는1부터 순환한다. 예를 들어 Throttle2개이면 Engine3의 Throttle:3은Throttle:1이 된다. 사용자 지정 프로젝트/기체 출력과 물리 모터 매핑은 유지한다. 기존 Wheel4개는 변경하지 않는다.

숫자 입력은 불필요한 소수점 뒤0을 제거해 1 / 0.5 / 0.1처럼 표시한다. 파라미터 행은 위쪽에 모아 고정 높이로 배치하며 창이 커져도 세로로 늘리지 않는다. 노면 선택에는 enum 드롭다운을 사용한다.

일반적인 기본 Lua 요청의150ms는 다음 입력까지 출력을 유지하려고 둔 예제 여유 시간이었고 게임/장치 규격값이 아니었다. 이를100ms로 줄였다. 입력 간격이100ms보다 길면 출력 사이에 공백이 생길 수 있으며 호스트가 자동 연장하지 않는다. 시동 이벤트/감쇠/Touchdown처럼 별도 길이를 계산하는 효과는 그 계산을 유지한다.

- Bank/Flaps/Overspeed/Stall/Shutdown/Startup을 포함해 Ground Roll 이외의 기본 PWM은 OFF다. 동적 강도 효과도 haptic_power 복사본의 strength만 바꾸고 그대로 전달한다.
- MSFS Ground Roll은Power1, PWM ON, 최소0.1knots/step0.1. X-Plane도 최소 속도 입력을knots로 제공하되 Lua에서 원본m/s를 변환한다.
- Touchdown 기본값은Minimum200ms, Maximum2000ms, Time multiplier2다. 첫 압축 상승 구간 판정은 유지한다.
- Ground Thumps는Power/Slab length/Minimum speed/Maximum speed/Pulse ms/Ground type만 제공하며 센터·좌·우 기어를 개별 출력한다. 주기 최소/최대 파라미터는 제거했다. 기본 슬랩4.6m는 [FAA AC150/5320-6G Table3-7](https://www.faa.gov/documentLibrary/media/Advisory_Circular/150-5320-6G-Pavement-Design.pdf)의15ft 간격을 참고한 예제값이며 모든 활주로의 실제 간격을 뜻하지 않는다.
- MSFS Thumps는 [ON ANY RUNWAY와 SURFACE TYPE](https://docs.flightsimulator.com/msfs2024/retail/programming-apis/simvars/aircraft-simvars/aircraft-miscellaneous-variables/)으로 활주로 여부와 Concrete/Asphalt를 검사한다. 미수신/미접지/속도 범위 밖에서는 출력하지 않는다. X-Plane은 [공식 dataref 목록](https://developer.x-plane.com/datarefs/)의 노면 재질을 사용하지만 유도로·계류장도 같은 재질이면 해당한다.
- 빠른 속도에서 요청이 겹쳐도 강도가 합산되지는 않지만 기한 갱신으로 연속 진동이 될 수 있다. Thumps는 가상 경계 통과 시에만 요청하고 펄스를 예상 간격의 절반 이하로 줄이며 휴지기를 둔다. 수신 주기·HID tick보다 짧은 충격은 생략되거나 구분되지 않을 수 있다. 동시에 선택된 다른 효과가 모터의 공백을 채울 수도 있다.
- MSFS2020에도 같은 변경을 적용했다. X-Plane은 추력 대신RPM, WT는HP 등 기존 게임별 입력을 유지한다. WT는 과속 한계와 바퀴/노면 입력이 없으므로 수동 과속 한계를 유지하고 지상 효과를 새로 만들지 않는다.

새 기본 Lua는 새 프로젝트 생성에 사용된다. 기존 프로젝트의 수정된 Lua와 저장된 Global/Specific 값은 덮어쓰지 않는다. 기존 프로젝트에 적용하려면 effects/<게임>/의 필요한 Lua를 직접 복사하고, 이전 파라미터 Override는 Reset to Default/Global로 정리한다.

## 정리된 포터블 폴더

루트에는 StellaVibra.exe(실행 런처), StellaVibra.pref, README.md, LICENSE를 둔다. bin에 실제 앱·PDB·Qt/MSVC DLL, bin/plugins에 Qt 플러그인, bin/simconnect/MSFS2024 및 MSFS2020에 버전별 공식 클라이언트를 둔다. 문서는 docs, 라이선스 원문은 licenses, 대응 소스는 sources, 기본 Lua는 effects, 진단 로그/덤프는 diagnostics다. 앱 설정·진단·템플릿은 작업 디렉토리가 아닌 이 배포 루트를 기준으로 찾는다. 기존 pref는 같은 위치에서 유지되며 bin 안의 실제 앱을 직접 실행해도 같은 설정을 사용한다. 배포 폴더 전체를 복사하거나 삭제하면 된다. 덤프 분석용으로는 당시 bin/StellaVibra.exe와 bin/StellaVibra.pdb를 보존한다. 폴더 재정리는 SimConnect 공개 재배포 권리 확인 여부를 변경하지 않는다(docs/SIMCONNECT-LICENSING.md 참조).

현재 버전은0.2.0이며 로컬 검증 단계다. DCS 연결은 다음 구현 대상이고 아직 지원하지 않는다. GitHub 연결·푸시·공개 배포는 DCS 연동과 충분한 실기 테스트 후 진행한다. 앱 버전과 프로젝트/pref의 파일 스키마 version1은 별개다.

앱 아이콘은 노란 네 갈래 별·하늘색 진동선·남색 배경이다. 승인된 원본은 assets/stella-vibra.png, Windows 다중 해상도 아이콘은 assets/stella-vibra.ico이며 scripts/make-icon.ps1로 원본에서 다시 생성할 수 있다. 루트 런처/실제 EXE와 Qt 앱 창에 포함하므로 배포 시 별도의 아이콘 파일은 필요 없다.

## 0.2.0 릴리스 정리 기준

기능 동결과 사용자 문서는 ../README.md, MANUAL.md, RELEASE-NOTES-0.2.0.md를 따른다. 이 파일의 이전 README/중간 구현 설명은 개발 배경으로 보존한다. --ui-check는 제거되었으며 DCS 후 버전 고정이라는 이전 순서는 최신 사용자 요청으로 대체되었다. 내부 운영은 ../WORKFLOWS.md와 RELEASE-CHECKLIST.md를 따른다.
