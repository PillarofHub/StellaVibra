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
