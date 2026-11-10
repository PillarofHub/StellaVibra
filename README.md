# Stella Vibra

Windows용 Lua 기반 햅틱 효과 편집·실행 도구. 제품 UI는 영어, 협업 문서는 한국어로 작성한다. 첫 MVP의 실행 코드와 Release 빌드가 구현되어 있다. GUI·MSFS·하드웨어의 실제 동작은 아직 검증하지 않았다.

이 문서는 현재 합의된 제품·실행 규칙의 기준이다. 작업 상태는 HANDOFF.md, 작업 순서와 새 채팅 프롬프트는 WORKFLOWS.md, 작업 제한은 AGENTS.md를 따른다. 세부 파일 스키마와 입력 필드는 CONTRACTS.md를 따른다.

## 기술과 MVP 범위

- C++20, Qt Widgets, CMake, MSVC, Windows 10/11 x64, 임베디드 Lua.
- Win32로 Windows/HID를 연동한다. DirectX는 필요할 때 사용하며 MVP UI에 별도 렌더러를 만들지 않는다.
- EXE와 필요한 DLL·플러그인을 동봉하는 포터블 폴더 배포. .NET 설치를 요구하지 않는다. Qt 6.8.3, Lua 5.4.8. 라이선스·배포 조건은 THIRD-PARTY-NOTICES.md를 따른다.
- MSFS 어댑터 하나, 확인된 하드웨어, Ground Roll과 Touchdown 두 Lua 효과부터 구현한다.
- 프로젝트·기체 프로필, Lua 편집과 자동 파라미터 UI, 논리 출력 매핑, 출력 모니터, 모터 테스트 UI, 로그를 포함한다.
- 완전한 IDE/LSP/스텝 디버거, 다중 게임 동시 연결, 범용 플러그인 시스템, 스크립트 패키지 관리자, 녹화·리플레이, 장치별 출력 보정은 제외한다.
- Aurora Vibra는 읽기 전용 참고 구현이다. GUI·게임·하드웨어 검증과 모터 구동은 사용자가 수행한다.

## UI

| 위치 | 화면 | 기능 |
| --- | --- | --- |
| 상단 | File / Edit / View 메뉴 | 프로젝트·저장 / Undo·Redo / 네 도킹 창 표시·숨기기 |
| 좌측 | Project Explorer | 단일 폴더 트리, Lua 파일 옆 활성화 체크박스, 접기·펼치기, 오류 표시 |
| 중앙 위 | Parameters | Global/Specific Aircraft 선택, 슬라이더·수치 입력, 출력 목적지, 모드별 개별/전체 Reset |
| 중앙 아래 | Script Editor | 파일별 탭, Lua/JSON 편집, 행 번호, 문법 강조, 검색, 저장 시 적용 |
| 우측 | Motor Monitor | 슬롯순 모터 목록, Slots 셀 매핑, HID 셀 테스트, Stop All/Resume All |
| 하단 | Debug Output | 효과별 로그·오류, 필터, Clear, 오류 위치 이동 |
| 최하단 | Status Bar | 게임·장치 연결과 실행 상태 |

Project Explorer, Effect Control, Motor Monitor, Debug Output은 도킹 창이며 View 메뉴에서 각각 표시·숨기기를 바꾼다. Effect Control 최상단 파일 탭 아래에 Parameters/Script Editor를 위아래로 묶는다. 두 영역 모두 선택한 파일에 속하며 분할선을 끌어 높이를 조절한다. 창의 X로 닫아도 View에서 다시 열 수 있고, 창을 숨겨도 실행은 중지되지 않는다. 창 배치는 로컬에 저장한다. 현재 출력에 적용되는 기체와 편집 중인 프로필을 분리한다. MSFS·지원 HID를 자동 연결하고 유효한 텔레메트리와 활성 효과가 있으면 자동 실행한다. Motor Monitor의 Stop All/Resume All이 전체 출력을 제어한다. 연결 자체나 테스트 창 열기는 진동 명령을 생성하지 않는다. 이전의 시작 시 Stopped 규칙은 이번 자동 실행 요청으로 변경되었다.

## 프로젝트와 설정

```text
My Flight Project/
  My Flight Project.sv
  effects/
    Landing/
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

스크립트 하나는 상태를 가진 효과 하나다. 호스트가 인스턴스의 init(api), update(context)를 호출한다. 상태는 self에 보관하며 destroy/exit 콜백 없이 호스트가 자원을 회수한다. 효과별 호출은 직렬화한다.

### 초기화

init은 파라미터·출력 목록을 등록하고 상태를 초기화한다. 비활성 효과도 UI 구성을 위해 초기화하되 출력은 허용하지 않는다. 등록 정의는 텔레메트리나 저장값에 따라 변하지 않는다. 등록이 끝나면 호스트가 글로벌·기체 값을 합성한다.

```lua
function effect:init(api)
    api:parameter {
        name = "Strength",
        type = "int",
        default = 20,
        min = 0,
        max = 255,
        step = 1
    }
    api:output("Left Gear")
    api:output("Right Gear")
end
```

parameter는 type="boolean"/"int"/"float"/"haptic_power"와 default를 등록한다. 숫자는 min/max/step이 필요하다. haptic_power는 strength, pwm, on_ms, period_ms를 묶으며 강도 기본 범위 0–255, 기본 PWM 시간 20/80ms다. 복합 기본값과 선택적 min/max를 스크립트가 지정한다(CONTRACTS.md). output은 표시명·호출 키·저장 키를 겸하는 문자열 하나다. 효과 내 이름은 고유해야 한다. 출력마다 UI가 논리 목적지 선택 행을 만들며 여러 목적지 또는 빈 목록을 허용한다. 출력 이름 변경은 설정 키 변경이므로 연결을 추측 이전하지 않는다.

### 업데이트

새 텔레메트리가 들어오면 update를 호출한다. 마지막 호출 후 일정 시간(초기 기준 100 ms) 동안 새 데이터가 없으면 최신 스냅샷으로 다시 호출한다. 이는 목표 호출 간격이며 하드 실시간 보장이 아니다.

context는 호출 정보와 호스트 API를 묶는다. 공식 예제는 context, telemetry, parameters 같은 풀네임을 사용한다.

| 필드 | 의미 |
| --- | --- |
| now_ms | 이번 호출의 호스트 단조 시각 |
| elapsed_ms | 이전 호출 이후 경과 시간 |
| telemetry | 최신 입력 스냅샷, 원래 샘플 식별자·시각 포함 |
| is_new_sample | 실제 새 입력인지 재전달인지 |
| telemetry_age_ms / telemetry_valid | 마지막 실제 수신 이후 경과 시간 / 입력 유효성 |
| parameters | 상속을 합성한 최종 파라미터 |

재전달은 원래 샘플 시각이나 입력 만료를 갱신하지 않는다. 첫 입력 전·누락값은 무효로 표현하며 0으로 위장하지 않는다. 게임 자체의 중복 패킷·시뮬레이션 시각 정체도 구분한다. 공통 텔레메트리는 단위와 의미를 고정하고 게임 전용 값은 별도 영역에 둔다. 필수 입력 미지원은 출력하지 않고 이유를 표시한다.

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
- 초기 HID 틱은 20 ms다. PWM 시간은 가까운 틱으로 반올림하고 중간값은 올림한다. 유효 시간은 UI에 표시한다. ON=0은 무출력, ON=주기는 연속 출력이다. 반올림으로 0이 된 ON을 몰래 늘리지 않는다.

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

- MSFS 2024 연결은 2초마다 재시도한다. Settings 없이 EXE 옆 공식 `SimConnect.dll`만 로드한다. 게임 내부 DLL이나 PATH를 검색하지 않는다.
- HID는 1초마다 검색하고 연결 유지 중인 장치 핸들은 재사용한다. 지원 목록은 아래 WINCTRL HID 표를 따른다. Aurora에 정의된 장치별 프로토콜만 사용하며, 미등록 PID에는 출력하지 않는다. 일련번호 없는/중복 장치는 세션 내 수동 매핑이 필요하다.
- 마지막 프로젝트를 시작 시 복원한다. 해당 파일을 못 찾으면 빈 화면이며 다른 최근 프로젝트를 대신 열지 않는다. File → Open Recent에서 최근 10개를 열 수 있다.
- Explorer는 파일 트리 하나다. 등록된 Lua 옆 체크박스는 Parameters에서 선택한 프로필의 enabled를 제어한다. 폴더를 접고 펼칠 수 있고, 파일 더블클릭으로 탭을 연다. 탭은 이동·닫기가 가능하며 미저장 파일/설정은 *로 표시한다.
- Parameters 상단에서 Global 또는 Specific Aircraft: TITLE을 선택한다. 실제 실행 기체는 수신된 TITLE로 결정된다. Global의 Reset All to Default는 선택 효과의 글로벌 활성화/파라미터/출력 설정을 지워 기본값(true/등록 기본값/빈 출력)으로 되돌린다. Specific의 Reset All to Global은 선택 효과의 해당 기체 Override 전체를 제거한다. 스크립트는 기체와 무관한 공통 파일이다.
- New Project는 빈 폴더에 Ground Roll, Touchdown을 만든다. 두 예제는 haptic_power를 사용한다. Left Gear→Throttle:1, Right Gear→Throttle:2, Center Gear는 기본 미배정이다.

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

Global에서는 Reset to Default, Specific에서는 Reset to Global로 상위 값을 상속한다. 상위 값과 같아진 enabled/파라미터/출력 항목은 메모리에서 제거하고 저장 시 `.sv`에서도 생략한다. enabled 기본값은 true, 출력 목적지 기본값은 빈 목록이다. 숫자/boolean/haptic_power의 기본값은 Lua 등록값이다. haptic_power는 전체 객체가 상속 단위이므로 숨겨진 PWM 시간도 비교에 포함한다. 빈 그룹과 빈 효과 Override도 제거하며 알 수 없는 설정 키는 보존한다. 기존 중복값은 의미를 유지한 채 일반 Save/Save All에서 정리된다.

수정 표시는 변경 이력의 존재가 아니라 저장 상태와의 차이를 뜻한다. 값을 1→2→1로 돌려 모든 설정이 저장 당시와 같아지거나 Lua/JSON을 실행 취소해 저장 텍스트로 돌아오면 별표와 저장 질문이 해제된다. 다른 파일/프로필에 변경이 남으면 해당 파일 또는 앱의 수정 표시는 유지한다.

Outputs는 출력 이름을 가로 제목으로 하는 표다. 아래 선택 셀에는 여러 목적지를 한 줄씩 표시하고, 셀에서 목적지를 선택한다. 출력별 출처/Reset도 같은 셀 안에 표시한다. 출력이 많으면 가로 스크롤한다.

## Lua 파워 직접 전달과 편집 보조

`context:vibrate(name, parameters.Power, duration_ms)`로 상속이 합성된 haptic_power 객체를 직접 전달한다. mode가 없으면 pwm boolean으로 Constant/PWM을 선택하며 기존 강도·시간 검증, 위상·기한 규칙을 그대로 적용한다. 객체를 변경하지 않는다. 명시적인 mode가 있으면 우선하고 기존 constant/pwm/off 헬퍼도 유지하므로 시간에 따른 강도 계산도 가능하다. 신규 프로젝트용 두 예제는 직접 전달을 사용한다. 기존 프로젝트의 Lua 파일은 자동으로 덮어쓰지 않는다.

File과 View 사이 Edit 메뉴는 활성 Lua/JSON 문서의 Undo(Ctrl+Z), Redo(Ctrl+Y 또는 Ctrl+Shift+Z)를 제공한다. 파라미터 변경 이력을 위한 Undo 스택은 추가하지 않는다.

Lua 편집기는 context: / api: 메서드와 context. / telemetry. / context.telemetry. 필드, telemetry.msfs. 및 gear. / telemetry.gears[index]. 필드를 자동 제안한다. 입력한 접두사로 필터하며 방향키·Enter/Tab으로 선택, Escape로 닫고 Ctrl+Space로 다시 요청한다. 문자열/주석과 JSON에서는 제안하지 않는다. 알려진 이름 기반의 가벼운 보조이며 임의 별칭·사용자 테이블·동적 파라미터명 추론이나 LSP는 제공하지 않는다.
