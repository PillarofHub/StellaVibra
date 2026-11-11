# Stella Vibra 인수인계

## 현재 상태 — 2026-11-10

첫 MVP 커밋 `3e9d4af` 이후 사용자 요청에 따라 IDE식 편집·자동 연결을 구현했다. 직전 IDE 작업 시작 시 트리는 깨끗했으며, 11월 10일 작업에서는 그 미커밋 변경 전체를 보존했다. 이번 후속 변경은 미커밋이며 푸시하지 않았다. Aurora를 변경하지 않았고 하위 에이전트·별도 채팅·앱 실행도 하지 않았다.

## 이번 변경

- Settings와 수동 Connect/Scan, Apply/Run/Stop 툴바 제거. File의 New/Open/Open Recent/Save/Save All/Close File/Close Project 제공. 파일별 탭으로 편집 대상을 전환한다.
- Ctrl+S는 현재 Lua와 해당 효과의 글로벌/기체 설정만 저장한다. 다른 효과의 파라미터·코드 초안은 유지한다. Ctrl+Shift+S는 전체 저장. 마지막 프로젝트 복원, 없으면 빈 상태, 최근 10개 목록.
- effects 하위 폴더를 1초 간격으로 추적한다. 같은 파일명으로 누락/신규 경로가 유일하게 대응되면 ID와 글로벌/기체 설정 유지. 대응 안 되는 누락 효과는 `.sv`와 모든 기체 Override에서 삭제. 새 이름은 기본값/빈 출력 매핑으로 등록. 구조 변경만 자동 저장하며 파라미터 초안은 저장하지 않는다.
- Explorer는 후속 작업에서 단일 디스크 트리와 Lua 체크박스로 통합했다.
- MSFS 연결은 2초 간격 재시도, 연결 중에는 SystemState 요청/응답으로 상태 확인(응답 제한 5초). 이 응답은 텔레메트리 시각을 갱신하지 않는다. HID는 1초 간격 탐색하며 기존 핸들을 재사용한다. 11월 10일 지원 목록을 Aurora의 11개 PID로 확장했다(아래 참조).
- 기본 자동 실행, Motor Monitor의 Stop All/Resume All. 유효한 입력·활성 효과·매핑이 있어야 출력한다. 프로젝트 전환은 출력을 정리하지만 사용자의 전체 중지 선택은 유지한다. 테스트 창 열기만으로 테스트 명령을 생성하지 않는다.
- 파라미터 수정 즉시 반영, 기존 Lua self/활성 PWM 위상 유지. 저장한 코드만 해당 효과를 교체하며 실패하면 해당 효과의 이전 유효 버전 유지. 다른 효과의 갱신은 계속한다.
- Parameters 상단 Global/Specific Aircraft: TITLE 및 Reset All to Global. 스크립트는 기체 공통이다.
- boolean/int/float 타입, 기본값/범위/step 검증, 숫자 슬라이더+직접 입력. haptic_power는 strength 정수(기본 0..255), PWM 체크, 조건부 ON/period 입력(기본 20/80ms). 강도 min/max와 복합 기본값을 Lua에서 지정한다. 복합값 전체가 하나의 Override 단위다.
- Ground Roll/Touchdown 새 프로젝트 예제를 haptic_power로 변환했다. 기존 프로젝트 Lua는 자동 덮어쓰지 않는다. Touchdown 첫 압축 상승 알고리즘은 유지했다.

## 빌드·배포

```powershell
.\scripts\build.ps1 -Deploy
```

MSVC 19.51.36260, Windows SDK 10.0.26100.0, Qt 6.8.3, Lua 5.4.8, VS Build Tools 2026 CMake. `.deps`와 빌드 산출물은 Git 제외.

- `E:\Workspace\Stella Vibra\build\Release\StellaVibra.exe`
- `E:\Workspace\Stella Vibra\dist\StellaVibra\StellaVibra.exe`

두 폴더에 Qt DLL/플러그인, app-local MSVC CRT, Lua 예제, 라이선스·Qt/Lua 소스, 공식 SimConnect.dll/출처/EULA를 배치한다. Settings를 제거하고 EXE 옆 DLL만 로드한다. SDK 클라이언트가 `.deps/msfs2024/client`에 없으면 배포 단계 실패. Git에는 Microsoft 바이너리/SDK 헤더/EULA PDF를 넣지 않는다.

## 검증 결과와 한계

1차 및 후속 Release 컴파일·링크·배포 성공. 최종 결과는 아래 완료 기록을 따른다. 공식 SDK 헤더에서 기존 C4245 경고가 나온다. windeployqt의 dxcompiler/dxil 등 안내는 Qt Widgets 앱의 정적 필수 import가 아니다.

앱/GUI, Lua 실행 테스트, MSFS 연결, HID 열거/프로브/모터 구동은 전혀 수행하지 않았다. 컴파일 성공을 실제 동작 검증 완료로 표현하지 않는다. 기체별 접촉/애니메이션 의미, 실제 연결·재연결, 좌우/출력 타이밍은 사용자 확인이 필요하다. 강제 종료·드라이버 장애에서 물리 모터 정지는 확인하지 않았다.

프로젝트 `.sv`/기체 JSON을 외부에서 동시에 편집하는 병합 기능은 없다. 외부 Lua 변경은 추적하며, 편집기에 미저장 코드가 있으면 그 초안을 유지하고 로그에 충돌을 알린다. 저장은 파일별 QSaveFile 원자 교체이며 전체 다중 파일 트랜잭션은 아니다. 동일 파일명이 여러 개라 대응이 모호한 이동은 설정을 추측하지 않고 삭제/신규 처리한다.

## 사용자 확인 순서

1. New → 두 예제 열기 → 각각 코드/파라미터 수정 → 한쪽 Save 후 다른 초안 유지 → Save All/Close/재시작 복원. 마지막 프로젝트 파일을 못 찾으면 빈 상태인지 확인.
2. Lua를 하위 폴더로 이동해 설정 유지, 이름 변경/삭제해 이전 글로벌·기체 설정 제거 확인.
3. Global/Specific Aircraft 선택, 슬라이더/수치, PWM/시간, 전체 Reset 확인. 저장 전 코드 미적용, 저장 후 해당 효과만 교체 확인.
4. MSFS 시작/종료, 지원 HID 연결/분리 시 자동 상태 갱신 확인. 정확한 TITLE·pause·입력 만료 확인.
5. 매핑·낮은 세기로 효과와 Stop All/Resume All 확인. Touchdown 첫 상승/재무장 상태가 파라미터 조절 중 유지되는지 확인.

## SimConnect 출처와 배포 조건

기존 공식 SDK Core 1.7.3에서 추출한 x64 원본을 사용한다. 84,480 bytes, SHA-256 `b10de7adf4c62e5f66c89dd6d01b64091bbbcec83411cfe191d6b85fbee61d15`. Aurora DLL은 복사하지 않았다. 현재 Release/dist는 로컬 실행 검증용 준비 폴더다. 공개 배포는 수행하지 않았으며 Microsoft DLL 공개 재배포 권리 확인은 기존 SIMCONNECT-LICENSING.md의 미해결 상태 그대로다. 이번 동봉 요청으로 그 권리가 확인되었다고 주장하지 않는다.

## 최종 완료 기록

- `scripts/build.ps1 -Deploy` 최종 실행 성공(exit 0), 로그 `build/ide-build-verified.log`. Release 컴파일·링크와 양쪽 폴더 배치 완료.
- `dumpbin /dependents`로 Release의 EXE/DLL 25개를 정적으로 검사했다. app-local 또는 Windows System32에서 찾지 못한 필수 import는 0개다. 원본/Release/dist SimConnect.dll SHA-256 일치를 확인했다.
- `git diff --check` 통과. 테스트·앱·MSFS·HID는 실행하지 않았다. 이번 변경은 커밋하지 않았다.
- 외부 삭제 시 효과 설정은 즉시 제거한다. 단, 열린 편집기의 미저장 코드 초안은 데이터 유실 방지를 위해 남긴다. 이를 Save하면 파일을 다시 만들고 자동 추적이 새 효과/기본값으로 등록한다. 삭제한 효과 설정을 복구하는 기능은 아니다.

## 2026-11-10 — View / Motor Monitor / 스틱 지원 / 빌드 출력

- View 메뉴에서 Project Explorer, Effect Control, Motor Monitor, Debug Output을 각각 표시/숨김. Parameters와 Script Editor를 Effect Control 도킹 창으로 통합했다. 닫힌 Explorer를 View에서 복구하며 모든 도킹 창의 상태를 저장한다. 숨김은 효과 실행을 중지하지 않는다.
- Monitor는 연결된 모터만 Motor/Slots/HID 세 열로 표시한다. Motor 열은 남은 폭을 채우고, HID는 52px·정수 0–255 중앙 정렬. Status/Connected 열 삭제. 매핑 예약은 유지한다. HID는 마지막 성공 전송 강도이며 UI 갱신 20ms, 장치 피드백이나 입력 축이 아니다.
- 사용자 신고 모델은 WINCTRL URSA MINOR Combat Joystick R. 기존 필터가 B970만 허용하여 모든 스틱을 제외한 것이 확인된 코드 원인이다. 사용자 기체 설정·Explorer 숨김과 무관하다. 실제 연결 PID는 프로브하지 않았다.
- Aurora CLAUDE/HANDOFF를 먼저 읽고 `src/hid/protocol.rs`, `worker.rs`, `win32.rs`만 읽기 전용 참고했다. 허용 PID: BC27–BC2C (Airbus/Combat·Fighter/Space L/R), B920/B930 (URSA 스로틀), B970 (Combat EX), BEA8 (Orion 스틱), BD64 (Orion 스로틀). 장치별 프레임과 output descriptor 선택을 코드 대조했다. 세부 바이트 계약은 CONTRACTS.md.
- 스틱 1채널은 Stick 슬롯, URSA 스로틀 Left/Right는 Throttle 슬롯. Orion 스로틀은 Combined 1채널에서 BF/CF 두 명령에 같은 선택 강도 전송. 파형 합산은 하지 않는다. B970의 저장 키와 A=Left/B=Right 유지. 알려지지 않은 PID 및 조회/접근 오류는 로그로 알린다.
- 부분 HID 전송 실패 후 재연결되면 필요한 0 보고서를 다시 시도하도록 출력 시도 이력을 유지한다. 실제 장치의 물리 정지를 검증했다는 뜻은 아니다.
- `scripts/build.ps1 -Deploy` 기본 출력은 성공 2줄, 전체 로그는 `build/build.log`. 실패는 오류 최대 12줄과 로그 경로, 종료 코드 1. `-Detailed`로 상세 콘솔 출력 가능. MSBuild `/verbosity:minimal /nologo`. 실패 경로는 정적 검토만 했으며 의도적인 실패 테스트는 실행하지 않았다.
- 기존 미커밋 작업을 보존했고 커밋/푸시는 하지 않는다. GUI·MSFS·HID 프로브·모터·동작 테스트는 이번에도 실행하지 않았다.

수동 확인: View에서 네 창을 끄고 켜고 재시작해 상태 유지 확인 → 스틱/스로틀 연결·분리와 Debug Output의 PID/오류 확인 → Stick 슬롯 매핑 후 효과 출력 목적지에 추가 → 낮은 강도로 HID 숫자·Stop/Resume 확인. 외부 파일 이동의 Explorer 표시와 기체별 파라미터는 사용자의 이전 미검증 항목으로 남아 있다.

검증 완료: 2026-11-10 최종 `scripts/build.ps1 -Deploy` exit 0, Release 컴파일·링크·두 폴더 배치 성공. 기본 빌드 출력이 성공 안내와 로그 경로 두 줄임을 확인했다. 최종 증분 빌드의 프로젝트 컴파일 진단 0개, Release/dist EXE 해시 일치, `git diff --check` 통과. GUI·장치 실제 동작은 미검증이다.

## 2026-11-10 — 단일 Explorer / 파일 탭 / raw JSON / 모니터 셀 조작

- 중복 효과 목록 제거. EffectFiles(QFileSystemModel 파생)의 Lua 체크박스로 현재 프로필 enabled 제어. 체크 갱신은 트리 재설정 없이 전달해 펼침 상태 유지.
- 파일마다 Editor를 별도로 생성하는 QTabWidget. 이동·닫기·미저장 *·경로 툴팁, 커서/스크롤/undo 보존. 파일 이동 추적 시 열린 탭 경로 갱신.
- 프로젝트/aircraft .sv를 파라미터 없이 raw JSON으로 열고 자동 생성 경고 표시. 문법/스키마/중복 기체 키/경로를 쓰기 전에 검증, 저장 후 메모리 프로젝트와 실행 설정 재반영. 디스크가 바뀐 초안은 덮어쓰기 거부.
- 파라미터 초안과 raw JSON 초안의 충돌을 막기 위해 파라미터 미저장 중 raw 편집은 읽기 전용, raw 미저장 중 파라미터/활성화 및 효과 구조 자동 반영은 보류. 저장/취소 후 재개. 다른 Lua 초안은 유지한다. 관리 대상이 아닌 .sv는 열람할 수 있으나 저장은 거부한다.
- Monitor 하단 Mapping/Test 버튼 제거. Slots 우클릭/더블클릭은 매핑, HID는 테스트. Stop All/Resume All 유지. 슬롯 카테고리/번호 → 장비명 → ID 정렬. 다중 슬롯은 가장 앞선 매핑, 미배정은 마지막. 모달창과 목록 재정렬은 장치 ID로 대상을 유지한다.
- 코드 변경: src/effect_files.h 신규, window.cpp/window_project.cpp/window.h, editor.cpp, model.cpp/model.h. 기존 미커밋 작업 전체 보존. 커밋/푸시·앱·테스트·HID 실행 없음.

수동 확인: Explorer의 폴더 접기/펼치기와 Lua 체크·프로필 전환 → 여러 Lua 탭의 undo/커서와 개별 Save → 프로젝트/기체 JSON의 정상 저장·잘못된 JSON 거부·초안 충돌 안내 → Slots/HID 셀의 대상을 확인하고 슬롯 변경 후 정렬 확인. JSON 파일은 자동 생성이므로 평소 Parameters를 이용한다.

이번 후속 검증: `scripts/build.ps1 -Deploy` 최종 exit 0, 프로젝트 컴파일 진단 0개, `git diff --check` 통과, Release/dist 실행 파일 해시 일치. 초기 컴파일에서 Qt의 slots 매크로와 정렬 변수명 충돌이 발생해 orderedSlots로 수정했다. 이 실제 실패에서 빌드 출력이 오류 12줄과 로그 경로로 제한되고 exit 1을 반환하는 것도 확인했다. 최종 로그는 build/build.log이며 GUI·JSON 저장 동작·탭/체크·모터 테스트 실제 실행은 하지 않았다.

## 2026-11-10 — Effect Control 파일 단위 배치

사용자가 파라미터와 스크립트의 소속 관계가 분리되어 보인다고 지적하여 파일 탭을 Effect Control 최상단으로 옮겼다. 그 아래를 Parameters(위)/Script Editor(아래)로 묶었다. 자주 조절하는 파라미터는 위에 유지하되 탭이 두 영역을 모두 제어하는 계층이다. 파일별 Editor/undo를 유지하며 QTabBar와 QStackedWidget의 선택·이동·닫기를 동기화한다. JSON 탭의 파라미터 숨김도 유지한다. 이전 미커밋 변경을 보존했다. 앱·테스트·MSFS·HID 실행과 커밋·푸시는 하지 않는다.

검증: scripts/build.ps1 -Deploy exit 0, 프로젝트 컴파일 진단 0개, git diff --check 통과, Release/dist EXE 해시 일치. 실제 GUI 배치와 탭 전환·이동·닫기는 사용자가 확인한다.

## 완료: 간결한 파라미터·출력 UI 및 최소 저장

중단 때 모델 compact(), 수정 비교/텍스트 기준 일부만 적용되어 있었고 UI 수정·빌드는 승인 검토 사용량 한도로 실행되지 않았다. 이번 재개에서 UI와 저장 처리를 완성하고 최종 빌드·정적 점검까지 완료했다.

- 파워 강도/PWM/ON/Period 가로 한 줄, PWM 시간 입력 68px, 강도 영역은 남은 폭 사용.
- Default/Global/Specific 출처 및 Global의 Reset to Default, Specific의 Reset to Global. 상속값과 같은 저장 항목 및 빈 그룹/효과 Override 제거. enabled 기본값 true/출력 기본값 빈 배열을 계약에 명시했다.
- 출력 이름을 열 제목으로 하는 헤더+선택 행 표, 한 목적지당 한 줄. 출력별 reset/출처도 셀에 제공.
- dirtyEffects는 현재/저장 설정을 최소 표현으로 비교하여 재계산. Lua/JSON 텍스트도 저장 기준으로 돌아오면 초안 제거. Windows CRLF 정규화를 기준 비교에 반영.
- README/CONTRACTS/WORKFLOWS 동기화 완료. scripts/build.ps1 -Deploy 최종 exit 0, 프로젝트 컴파일 진단 0개, git diff --check 통과, Release/dist EXE 해시 일치. 로그 build/build.log. 앱/테스트/MSFS/HID 실행 및 커밋·푸시 없음.

사용자 확인 순서: Power의 PWM on/off 시 한 줄 배치와 숫자 입력 → Global Reset to Default/Specific Reset to Global과 출처 표시 → 1→2→1 복귀 시 탭 별표·종료 저장 질문 해제 → Lua/JSON 편집 Undo로 저장 텍스트 복귀 → Save 후 중복 파라미터/Override 삭제 및 enabled=false·빈 출력의 필요한 Override 유지 → 여러 출력/다중 목적지 셀 확인. GUI·하드웨어 동작은 미검증이다.

일반 Save/Save All은 모든 읽을 수 있는 Lua 기본값을 갱신해 최소 표현으로 저장한다. Lua 오류가 있으면 기본값을 추정하지 않고 해당 설정을 보존한다. raw JSON 개별 저장은 원시 편집 내용을 유지하고 다음 일반 저장에서 중복값을 정리한다. 비교 기준은 설정의 최소 표현과 편집기의 줄바꿈 정규화된 저장 텍스트다. 값 복귀 후에도 다른 파일/프로필 변경이 남으면 전체 수정 표시가 유지된다.

## 2026-11-10 — 모드별 전체 Reset / 파워 직접 전달 / 편집 보조

사용자가 이전 간결한 파라미터 UI 적용을 확인했다. 이번 변경은 Global의 Reset All to Default(선택 효과의 enabled/parameters/outputs 제거), Specific의 Reset All to Global(해당 효과 Override 전체 제거)이다. 기본 출력은 빈 목적지이므로 Global 전체 Reset은 출력 매핑도 초기화한다.

vibrate는 mode 없는 haptic_power 객체의 pwm boolean으로 Constant/PWM을 선택한다. 명시적 mode 우선, 기존 검증·파형·기한 및 constant/pwm/off 유지, 전달 객체 불변. 번들 Ground Roll/Touchdown은 parameters.Power를 직접 전달하고 게임 판단 알고리즘은 변경하지 않았다. 기존 사용자 프로젝트는 자동 수정하지 않는다.

Edit 메뉴에 현재 Lua/JSON 문서 Undo(Ctrl+Z), Redo(Ctrl+Y/Ctrl+Shift+Z)를 추가했다. 파라미터 Undo는 범위 밖이다. Editor에 QCompleter를 붙여 context:/api:, context./telemetry., msfs와 gear 필드를 제안한다. 접두사 필터·Enter/Tab 선택·Esc 취소·Ctrl+Space 재요청을 제공한다. Lua 문자열/주석/긴 괄호와 JSON에서는 제안하지 않는다. 알려진 이름 기반이며 임의 별칭/사용자 객체 타입 추론이나 LSP는 아니다.

검증: scripts/build.ps1 -Deploy 최종 exit 0, 컴파일·링크 및 Release/dist 갱신 성공, git diff --check 통과. 실행 파일 경로 build/Release/StellaVibra.exe, dist/StellaVibra/StellaVibra.exe. 앱·Lua 동작 테스트·MSFS·HID 실행과 커밋/푸시는 하지 않았다. 기존 미커밋 변경 보존. 초기 편집 도구 호출에서 Windows python 별칭 실행이 실패하여 PowerShell/패치로 진행했으며 해당 실패 호출은 파일을 변경하지 않았다.

수동 확인: 프로필 전환 시 Reset All 제목과 선택 효과의 초기화 범위 → Lua/JSON 편집 후 Edit/단축키 Undo/Redo 및 저장 상태 별표 → context:와 telemetry. 입력 후 목록·선택·취소 → 새 프로젝트 예제에서 Power PWM on/off와 상속 반영. GUI·런타임 동작은 사용자 확인 전이다.

## 2026-11-11 — 기능별 선행 커밋 / Lua 계약 분리 / 효과 카테고리와 진단 UI

사용자 지시에 따라 신규 작업 전에 누적 변경을 네 커밋으로 나눴다: af17f76(자동 MSFS/HID), c5d1328(typed power/상속 실행), 4d9e669(IDE/Explorer/파라미터 UI), 42f7995(조용한 배포/문서). 푸시 없음. 이번 신규 변경은 작업 트리에 남기며 별도 커밋 요청으로 해석하지 않았다.

사용자는 도중에 하위 호환 불필요를 명시했다. 최종 계약은 init(context), update(context, telemetry, parameters), context는 메서드만이다. init에 parameter/output/log, update에 log/vibrate/now/elapsed/sample_is_new/input_valid. context.telemetry/context.parameters/과거 데이터 필드와 type 생략 추론을 제거했다. parameters는 원래 name에서 공백을 _로 치환한 접근 키만 제공한다. ASCII Lua 식별자/예약어/충돌을 init에서 검증하고 저장·표시 키는 원래 name을 유지한다. 기존 사용자 Lua는 자동 변환하지 않아 이전 update 스크립트는 수동 수정 또는 새 프로젝트 생성이 필요하다.

자동완성: phase별 context 메서드, 실제 지원 telemetry 필드, init의 리터럴 name/type에서 parameters 키와 haptic_power 하위 필드. 계산된 등록 이름·별칭·전체 언어 분석은 범위 밖이다. src/telemetry.h의 공통 목록으로 SimConnect 등록/디코딩/완성/값 뷰를 일치시켰다. 필수9 + 항공역학/엔진 부가 입력 총48개 FLOAT64 및 TITLE 구독. 표준 SimVar가 0으로 반환하는 미지원 애드온과 진짜0을 구별할 수 없으며 L-var 보정은 이식하지 않았다. 공식 MSFS 2024 SimVars/engine 문서 및 Aurora src/sim/elem_idx.rs, src/rumble.rs, src/ui.rs를 읽기 전용 참고했다.

Explorer 오른쪽 On 열: effects/하위 폴더 부분 체크 및 일괄 활성화, 개별 Lua 체크. .sv/aircraft는 체크 없음. 선택 프로필에 batch 반영한다. Parameters 상단 Enabled/Disabled 버튼으로 특별 활성화 상태를 분리했다. 기존 설명문/기본 enabled 행 제거, Power의 전용600px 상한 제거. New Project 대화상자는 디렉토리/탐색, MSFS2024 라디오, 기본 Lua 생성 체크를 제공하며 해제 시 빈 effects/aircrafts만 생성한다.

번들 11개: ground(ground_roll, touchdown, ground_thumps), aerodynamics(stall, overspeed, spoilers, flaps, bank), engines(startup, shutdown, thrust). Touchdown 첫 압축 상승 알고리즘 유지. Ground Thumps는 속도/슬랩 간격 모델, Aero는 Aurora 표준 조건 참고, 엔진은 1..4 표준 starter/combustion edge 및 jet/prop 추력을 사용한다. 엔진 최초 샘플에서 edge를 만들지 않는다. Thrust의 Propeller thrust와 Full scale pounds는 기체별 조절이 필요하다. Airframe 기본 Stick:1, Engine N은 Throttle:N, Left/Right Gear는 Throttle:1/2, Center Gear 미배정이다. Aurora 고지 유지, 파형 혼합/기체 전용 보정/과거 강도 곡선은 가져오지 않았다.

Telemetry 도킹 창을 Motor Monitor 아래 추가했다. 지원 값·단위/원 SimVar 툴팁, 미수신 Unavailable, 연결/Paused/Stale와 age 표시. UI만100ms 갱신하며 received_ms를 연장하지 않는다. View 마지막 Reset Default Layout은 모든 창을 보여주고 기본 배치로 복원. Settings/Set Tick: HID/PWM5..100ms, 기본20, OK/Cancel/Reset, 로컬 저장. 틱 변경 시 요청/테스트 제거·self 재초기화. PWM 양자화는 HID 후보 비교 전 공통 tick 적용, period/on이0으로 반올림되면 무출력. Help/About에서 MIT/Qt LGPL/Aurora와 영문 타사 안내 제공.

컴파일 검증: scripts/build.ps1 -Deploy로 C++ 컴파일·링크와 CheckLua(LuaCompiler -p)를 수행했다. luac는 구문 컴파일만 하며 init/update를 실행하지 않는다. CMake에 기본 효과 문법 컴파일 target을 추가했다. 배포 시 이전 루트 ground_roll.lua/touchdown.lua 두 번들 파일만 제거하여 새 ground/ 예제와 중복되지 않게 했다. 모든 사용자 프로젝트와 Aurora 저장소는 수정하지 않았다. GUI·Lua 실행 동작·MSFS·HID·모터·테스트는 실행하지 않았다.

수동 확인: (1) 새 프로젝트 기본 예제 생성 ON/OFF 및11개 카테고리 트리 (2) 폴더 부분체크/일괄 토글·Enabled 버튼·기체 상속과 저장 (3) context:/telemetry./parameters./parameters.Power. 자동완성, Undo/Redo (4) View 복원·Telemetry Live/Paused/Stale와 실제 단위 (5) Set Tick 취소/적용/20ms 초기화/재시작 지속 (6) About 고지 (7) 매핑과 낮은 강도로 개별 Ground/Aero/Engine 조건을 사용자가 검증. 복수 효과는 합산하지 않으므로 다른 효과를 꺼서 개별 검증한다.

최종 검증 결과: scripts/build.ps1 -Deploy exit 0, 프로젝트 컴파일 진단0개, CheckLua의 11개 Lua 구문 컴파일 성공, git diff --check 통과. Release/dist 각각 예제11개이며 EXE SHA-256 일치. build/Release/StellaVibra.exe와 dist/StellaVibra/StellaVibra.exe 및 필요한 DLL/플러그인/라이선스/예제를 갱신했다. GUI·MSFS·HID·Lua 콜백 동작은 실행하지 않아 미검증이다.

## 2026-11-11 — 시작 직후 도킹 레이아웃 충돌 수정

사용자가 실행 실패를 신고했고 이후 필요시 직접 실행을 허용했다. Windows Application 오류와 기존 사용자 CrashDumps를 읽었다. Qt6Widgets.dll+0x163175 접근 위반(c0000005), 같은 레이아웃 경로의 fail-fast(c0000409)를 확인했다. 실제 사용자 local.json은 layout_version=2였다. 새 코드가 저장 배치를 먼저 restoreState한 뒤 버전 불일치로 기본 배치를 즉시 다시 restoreState하여, 창 표시 전 Qt에 남은 레이아웃 참조가 무효화되는 경로였다.

수정: 시작 시 버전3인 저장 배치만 한 번 복원한다. 이전 버전/없는 배치는 새로 구성한 기본 배치를 그대로 유지한다. 사용자 local.json이나 프로젝트 파일을 삭제/수정하지 않았다. View/Reset Default Layout은 창이 표시된 뒤 정상 동작한다.

사용자가 허용한 직접 실행 확인을 위한 --ui-check 옵션을 추가했다. Engine 스레드를 생성하지 않아 MSFS/HID 검색·모터 출력을 하지 않고, 프로젝트 자동 복원과 로컬 설정 저장을 차단한다. 창 표시 후 기본 배치 복원 action을 호출하고 2초 뒤 종료한다. Engine 소멸자는 시작된 스레드만 join한다. 일반 실행의 자동 연결 동작은 유지한다.

검증: 동일 사용자 설정에서 수정 전 --ui-check는 constructed 뒤 종료코드 -1073740791로 재현, 수정 후 창 표시/default layout 복원/event loop responsive 로그와 exit0 확인. scripts/build.ps1 -Deploy exit0, C++ 컴파일·링크 및 Lua 구문 컴파일 통과. 앱 실제 시작을 확인했지만 일반 모드 MSFS/HID 연결 및 기존 프로젝트 자동 복원은 이번 실행 점검에서 제외했다. Release/dist 실행 파일 갱신, 커밋/푸시 없음. 진단 로그 build/startup-check.log, build/startup-check-fixed.log. 기존 덤프나 Aurora는 변경하지 않았다.

## 2026-11-11 — 기본 비율 / 이름 뒤 체크 / effects 자동 펼침

중단된 작업을 이어 완성했다. 기본 좌우1:4:1, 본문/Debug Output4:1, Parameters/Script1:1. Monitor 최소폭380 제거(120으로 완화), Output 목록 최소높이24 및 Ignored 수직정책으로 과도한 최소높이를 줄였다. layout_version4에서 사용자 배치를 저장하며 이전 버전은 구성된 기본 배치 유지 후 이벤트루프에서 비율만 적용한다. 이전 시작 충돌 원인인 창 표시 전 연속 restoreState는 다시 도입하지 않았다. Reset Default Layout도 동일 비율을 적용한다.

EffectCheckDelegate/EffectTree로 별도 On 열을 없애고 이름 바로 뒤에 체크박스를 그린다. 클릭 판정과 Space 토글, 그룹 부분 체크, 체크박스 더블클릭 시 파일 열림 방지를 포함한다. 프로젝트 open 때 effects와 모든 하위 폴더를 펼치고 QFileSystemModel directoryLoaded로 비동기 로딩을 처리한다. 한번 펼친 항목을 대기집합에서 제거하므로 이후 사용자가 접은 폴더를 지속적으로 강제 펼치지 않는다. aircraft/.sv는 체크 없음.

텔레메트리 검토: 현재48개 숫자/boolean+TITLE는 공식 전체 목록이 아닌 명시적 구독 부분집합이다. 공식 AddToDataDefinition/RequestDataOnSimObject/EnumerateInputEvents 문서 및 포함 SDK1.7.3 헤더를 확인했다. 연결만으로 SimVar가 자동 추가되지 않으며 InputEvents 열거는 별도 인터페이스다. 미지원/미갱신/정상0을 값만으로 구분할 수 없어 자동완성에서 임의 삭제하지 않는다. 현재 구독 계약을 유지하고 README에 근거 링크와 한계를 기록했다. 게임 연결이나 미지원 변수 프로브는 실행하지 않았다.

검증: scripts/build.ps1 -Deploy 성공, C++/Lua 구문 컴파일 및 diff 점검. 기존 사용자 허용에 따라 --ui-check 직접 실행: MSFS/HID/모터/프로젝트 복원/설정 저장 차단, exit0. 최종 창 치수 Explorer345x852, EffectControl1356x852, Monitor339x424, Telemetry339x424, Output2048x219, 내부 splitter412/411로 요청 비율에 근접함을 확인했다. 로그 build/layout-check.log. 초기 qInfo 스트림 로그가 표시되지 않아 printf형 qInfo로 바꿔 실제 치수를 확인했다. 실제 프로젝트의 체크박스 클릭/키보드/펼침은 사용자가 확인한다. Release/dist 갱신, 커밋/푸시 없음. 이전 모든 미커밋 변경 유지.

## 2026-11-11 — 아이콘 앞 체크박스 / 추가 텔레메트리 검토

사용자가 앞선 동작을 확인했으나 이름 뒤 체크 정렬이 불편하다고 요청했다. custom paint/sizeHint/editorEvent를 제거하고 Qt 기본 item delegate 배치(들여쓰기→체크→아이콘→이름)를 사용한다. EffectCheckDelegate는 native checkbox rect 조회만 남겨 EffectTree의 체크 더블클릭 파일열기 방지를 유지한다. effectGroup은 effectsRoot 자체를 제외한 하위 디렉토리만 반환하여 최상위 effects에는 표시/조작 모두 체크가 없다. 하위 그룹 부분체크·일괄토글·Space, 파일 개별체크·자동 펼침은 유지한다.

현재11개 Lua의 입력 사용을 대조했으며 현재 구독으로 코드상 요구는 충족한다. 공식 MSFS 문서 검토 결과 추가 우선순위: SURFACE TYPE(노면 분류), G FORCE/ACCELERATION BODY XYZ(하중·흔들림), INCIDENCE ALPHA/STALL ALPHA(실속 접근), GEAR TOTAL PCT EXTENDED/BRAKE LEFT·RIGHT POSITION(기어·제동). 값의 물리적 한계와 Lua 필터링/기체별 검증을 README에 기록했다. 기존 RPM%/N1/N2 재사용도 권장한다. 검토 요청이므로 구독·자동완성·효과 알고리즘은 추가/변경하지 않았다. Touchdown 첫 압축 상승 유지.

검증은 C++ 컴파일·링크, 기존 Lua 구문 컴파일, diff와 배포 일치 점검이다. 이번에는 앱/테스트/MSFS/HID/모터를 실행하지 않았다. 사용자 확인: effects 루트에 체크 없음 → 하위 폴더와 Lua 아이콘 앞 체크 정렬 → 그룹 부분체크와 Space/마우스 토글. 기존 미커밋 작업 보존, 커밋/푸시 없음.

최종 build.ps1 -Deploy exit0, 프로젝트 컴파일 진단0개, git diff --check 통과, Release/dist EXE 해시 일치. README 검토 결과도 두 배포 폴더에 갱신했다.
