# 단위 작업 운영

## 현재 범위

첫 MVP W1–W6은 `3e9d4af`에 커밋되었다. 이번 후속 작업은 IDE식 File 메뉴와 독립 저장, 마지막 프로젝트 복원, 외부 effects 파일 이동/삭제 추적, 자동 MSFS/HID 연결, 즉시 파라미터 반영, 저장 시 스크립트 갱신, Parameters의 기체 선택과 typed/haptic_power UI다. 이번 수정은 커밋·푸시하지 않는다.

## 구현 순서와 검증

1. 기존 문서·git status 확인, 기존 구현 보존.
2. 파라미터 계약과 모델/Lua 검증 확장, 두 예제 변환.
3. 실행 상태 보존과 효과별 교체, 자동 연결/핫플러그 구현.
4. File 메뉴·독립 저장·복원·파일 추적·파라미터 UI 반영.
5. Release 컴파일·링크, 로컬 Release/dist에 공식 SimConnect.dll과 의존성 배치, diff 점검.
6. README/CONTRACTS/HANDOFF 동기화.

사용자의 추가 지시에 따라 사라진 파일의 복구 설정은 남기지 않는다. 같은 파일명으로 대응되는 이동만 설정을 이전하고, 나머지 누락 효과와 기체 Override는 삭제한다. 코드 실행 상태와 저장된 초안은 구분한다.

GUI·MSFS·HID·Lua 동작 테스트는 수행하지 않는다. 사용자는 README의 수동 검증 순서를 따른다. 관측한 오류의 Output 메시지·기체 TITLE·장치 ID를 근거로 후속 수정 범위를 정한다. Aurora는 읽기 전용이며 별도 채팅·하위 에이전트·세션 일지를 만들지 않는다. 공개 바이너리 배포 권리는 SIMCONNECT-LICENSING.md의 기존 검토 상태를 따른다.

## 2026-11-10 추가 범위

View 도킹 창 토글 → Motor/Slots/HID 모니터 정리 → Aurora의 11개 스틱/스로틀 PID·프레임 이식 → 기본 빌드 출력 축소를 구현한다. 기본 빌드는 `.\scripts\build.ps1 -Deploy`로 호출하고 결과 두 줄만 읽는다. 실패할 때 `build/build.log`에서 해당 오류 범위만 읽고, 필요할 때만 `-Detailed`를 사용한다. 기존 미커밋 변경 보존, 컴파일·링크·diff만 검증하며 장치 프로브/테스트/앱 실행은 하지 않는다.

후속 범위: 단일 파일 트리 체크박스, 문서별 탭, 프로젝트/기체 raw JSON 검증 저장, 모니터 셀 조작과 슬롯 정렬. 빌드 출력 최소화 및 컴파일 전용 검증 규칙은 유지한다.

Effect Control 배치 후속: 파일 탭을 최상단, 그 아래 Parameters/Script Editor 분할로 구성한다. 개별 편집 문서를 유지하며 탭 선택·이동·닫기의 인덱스 동기화를 정적으로 점검하고 컴파일한다.

파라미터 후속: Power 한 줄/Outputs 가로 표, Default→Global→Specific 최소 저장 및 값 복귀 시 수정 표시 해제. 일반 저장 전 Lua 기본값 갱신·중복값 제거, 알 수 없는 키 보존. 검증은 기존대로 컴파일·diff에 한정한다.

편집 보조 후속: 모드별 Reset All, haptic_power의 vibrate 직접 전달과 예제 단순화, Edit Undo/Redo, 알려진 Lua API/텔레메트리 멤버 자동완성. 기존대로 컴파일·링크·diff만 검증하고 Release/dist를 갱신한다.

## 2026-11-11 범위

선행 누적 변경을 사용자 요청대로 af17f76/c5d1328/4d9e669/42f7995 네 기능 커밋으로 기록했다. 이번 신규 구현: 호환 분기 없는 Lua 세 인수/API context와 파라미터 alias 완성, 폴더 오른쪽 체크/Enabled 상태, 새 프로젝트 옵션, 세 카테고리 11개 효과, 공식 표준 SimVar 확장/Telemetry 창, View 복원, Settings Tick, About 고지. C++ 컴파일·링크와 luac -p 문법 컴파일만 수행하며 Lua 콜백/앱/게임/HID 테스트는 실행하지 않는다. 이번 신규 변경은 선행 작업 커밋 요청에 자동 포함하지 않고 작업 트리에 둔다.

레이아웃 후속: 1:4:1 좌우, 4:1 본문/Output, 1:1 Parameters/Script, 이름 뒤 체크박스, open 시 effects 전체 펼침. 공식 SimConnect 문서/헤더 검토 결과 구독 목록 기반 자동완성 유지. 하드웨어 없는 --ui-check로 시작·배치 복원을 점검하고 실제 트리 조작은 사용자 확인으로 남긴다.

체크박스 후속: Qt 기본 아이콘 앞 배치로 단순화하고 최상위 effects 체크를 제외한다. 텔레메트리는 기존11개 효과의 입력 충족 여부 및 추가 후보만 검토하며 구독 확대는 하지 않는다.

## 게임 확장 후속 (2026-11-11)

사용자가 수락한 누적 UI·Lua 작업을 신규 구현 전에 d95db7e / 2d08eb0 / eb08984로 기능별 커밋했다. 이후 요청 범위는 추천 MSFS 입력 10개 추가, 프로젝트별 MSFS2020/XPlane12/WarThunder 연결, 게임별 원본 텔레메트리·완성·뷰·기본 Lua, 없는 디렉토리 생성과 기존 폴더 확인이다. 새 변경은 선행 커밋 요청과 구분하여 작업 트리에 남긴다. 푸시하지 않는다.

Aurora의 xp_link/datarefs.rs·rref.rs, wt_link/http.rs·vars.rs·worker.rs, sim/worker.rs를 읽기 전용 참고한다. 게임 공통 FlightVars 변환이나 Aurora 파형 혼합은 가져오지 않는다. 공식 문서와 SDK ABI를 우선하며 게임별 원본 입력을 Lua에 전달한다. 공식 2020 SDK DLL은 설치 없이 추출해 로컬 Release/dist에 배치한다. 전체 변수 무제한 구독 대신 MSFS58+TITLE, XP85 RREF, WT 두 엔드포인트 전체 JSON을 사용한다.

검증은 `.\scripts\build.ps1 -Deploy`로 C++/링크와 전체 Lua `luac -p`, `git diff --check`, 배포 파일/해시 확인이다. 앱·MSFS·X-Plane·War Thunder·HID 프로브·모터·런타임 테스트는 실행하지 않는다. 실제 게임 종료/재시작·pause·기체 변경·native 자동완성·폴더 덮어쓰기 확인은 사용자가 수행한다.

## 실행 폴더별 설정 후속

localPath를 applicationDirPath/StellaVibra.pref로 변경한다. 기존 JSON 스키마와 QSaveFile 원자 저장은 유지하며 AppData 자동 이전/대체 저장은 추가하지 않는다. 기존 파일을 삭제하거나 사용자 설정을 배포 파일로 복사하지 않는다. README/CONTRACTS/HANDOFF에 이전 위치, 선택적 수동 이전, 초기화·폴더별 분리·쓰기 권한을 기록한다. 컴파일·링크·diff와 Release/dist 배치만 확인하며 앱 실행/테스트/커밋은 하지 않는다.

## 타이밍 설정 후속

Set Tick을 Set Timings로 변경하고 HID tick과 Maximum update period를 함께 편집한다. 기존 설명문 제거, 두 값 기본값 복원, OK/Cancel, pref 저장·시작 복원을 연결한다. 기존100ms 재전달 조건만 설정값으로 대체하며 새 입력 처리/게임 구독/유효기간은 유지한다. 컴파일·링크·diff 점검과 Release/dist 갱신을 수행하며 앱·게임·HID·런타임 테스트·커밋은 하지 않는다.

## Lua 출력 기본값 후속

output(name, default_slots) 등록 → 슬롯 배열 검증 → configure 기본값 합성 → UI/cacheDefaults/리셋/최소 저장/선택창 합성 → 새 프로젝트의 이름 기반 하드코딩 제거 순서로 구현한다. 모든 게임의 기본 Lua output 등록은 언롤링하고 Center Gear=Stick:1, Left/Right=Throttle:1/2 등 기존 의도를 스크립트에 명시한다. 출력 처리와 상태 초기화만 수행하는 반복문은 유지한다. 기존 사용자 프로젝트 파일을 변경하지 않는다. 컴파일·Lua 구문 컴파일·diff·배포 해시만 확인하며 앱/런타임 테스트/커밋은 하지 않는다.

## 새 프로젝트 Browse 후속

Qt6.8.3 로컬 qfiledialog.cpp의 Directory 모드가 없는 폴더를 거부하고 버튼도 비활성화하는 것을 확인했다. 별도 폴더 트리/경로 입력 선택창에서 Select Folder 시 mkpath로 자동 생성하고, New Project에서는 빈 폴더 확인을 생략한다. 숨김/시스템/하위 폴더가 있으면 내용 보호 확인은 유지한다. 컴파일·diff·배포만 확인하고 앱/GUI 테스트는 사용자가 수행한다.

## 크래시 조사 후속

사용자가 dist에서 앱→MSFS 시작만으로 종료된 상황을 보고했다. Windows 이벤트1000/1001과 WER를 읽기 전용으로 확인한다. 17:15:11 접근 위반·모듈unknown이며 당시 덤프는 남아 있지 않다. DLL 수명 위험 보완(PIN)과 app-local 진단 로그/최선의 미니덤프/PDB를 추가하되 원인 확정·재현 성공으로 표현하지 않는다. 앱·게임·HID·고의 크래시 테스트 없이 컴파일/링크/diff/배포만 검증한다. 다음 사용자 재현은 Stop All→MSFS 시작으로 연결/출력 단계를 분리한다.

2026-11-11 배치 후속: 전체 창 리사이즈 시 사용자 도크 비율 유지, 기본 본문/Debug Output 3:1로 변경, 도크/내부 분할 최소 크기 완화. 빌드·배포·diff만 확인하고 실제 축소/확대·최대화·수동 분할선 조절·배치 복원은 사용자 확인.

## 기본 Lua·채널 수·배포 정리 (2026-11-12)

MSFS2024 기본값 수정 후 MSFS2020/XPlane12/WarThunder의 대응 효과를 수정한다. 노면 자료는 MSFS 공식 SimVar 문서와 X-Plane 공식 datarefs.json, 슬랩 예시는 FAA AC150/5320-6G Table3-7을 확인한다. 추정 센서·게임 공통 텔레메트리는 만들지 않는다. Ground Thumps는 바퀴별 거리 누적/경계 이벤트와 휴지기를 사용한다. 노면 선택용 enum을 Lua/검증/UI/저장에 추가하고 채널 수는 pref→Bundle→configure/UI 동일 기준으로 전달한다. Qt 숫자 표시의 후행0 제거와 파라미터 상단 고정 배치를 적용한다.

배포 루트 Win32 런처→bin 실제 앱으로 분리하고 pref·템플릿·고지·진단은 배포 루트를 사용한다. Qt/MSVC/bin/plugins 및 bin/simconnect 버전별 배치를 구성한다. 이전 생성 파일만 대체 파일 존재 확인 후 정리하며 사용자 파일은 보존한다. 컴파일/링크/luac 구문 컴파일/diff/PE 의존성/배포 해시만 확인한다. 앱·게임·HID·런타임 테스트·커밋·푸시는 하지 않는다.

0.2.0 체크포인트: CMake/앱/About/EXE 버전 통일, 빌드·diff 후 현재 누적 소스 변경을 사용자 요청의 단일 로컬 커밋으로 기록한다. GitHub 연결·푸시·배포는 DCS 연결과 충분한 실기 검증 후 별도 지시까지 보류한다. DCS는 이번에 연동 설명만 하며 기존 Export.lua 수정이나 실제 구현은 하지 않는다.
