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
