# 0.2.0 작업 및 릴리스 운영

## 현재 기준

사용자용 영문 문서: README.md, docs/MANUAL.md, docs/RELEASE-NOTES-0.2.0.md, docs/THIRD-PARTY-NOTICES.md.
내부 설계: docs/DEVELOPMENT.md(기존 한국어 README), CONTRACTS.md. 구현 이력·검증 결과: HANDOFF.md. 라이선스 상세 검토: SIMCONNECT-LICENSING.md. AGENTS.md의 작업 규칙을 먼저 따른다.

현재 기능을0.2.0으로 고정한다. DCS는 이 릴리스에 포함하지 않는다. 이전의 “DCS 후에만 버전 고정” 순서는 사용자의 최신 요청으로 대체되었다. GitHub 연결·푸시·공개 업로드는 이번 작업에서 하지 않는다. 공식 SimConnect 클라이언트의 공개 재배포 권리 확인은 별도 미해결 항목이다.

## 변경 및 검증

1. git status와 기준 문서 확인, 사용자 작업 보존.
2. 필요한 코드/문서만 변경. 임시 레이아웃 --ui-check 경로는 정리했으며 일반 진단 로그/덤프는 유지한다.
3. `.\scripts\build.ps1 -Deploy`로 C++/리소스 컴파일·링크와 Lua 구문 컴파일, Release/dist 갱신. 요약 출력만 읽고 오류가 있을 때 build/build.log를 확인한다.
4. diff 점검, 문서 상대경로·배포 해시·버전 리소스 확인. 앱/게임/HID/모터/런타임 테스트는 명시적 요청 없이 실행하지 않는다.
5. HANDOFF 갱신. 커밋은 현재 작업에서 요청받았을 때만 파일을 선택해 수행하고 푸시하지 않는다.

Help → README/User Manual은 배포 루트의 파일을 QTextBrowser에서 열며 로컬 링크·이미지는 현재 문서 기준으로 찾는다. 초기 파일이 없으면 오류를 보여준다. 별도 브라우저/Markdown 연결 없이 읽을 수 있고 웹 링크는 기본 브라우저로 연다. CMake install에서 README, 사용자 문서, 로고를 함께 배치한다. 한국어 내부 기획/세션 문서는 사용자 배포에 넣지 않는다.

## 공개 릴리스 전 남은 작업

- docs/RELEASE-NOTES-0.2.0.md의 수동 인수 항목을 실제 GUI·게임·장치에서 확인하고 결과를 기록한다. 기존 MSFS2024 이착륙 성공과 최종 패키지 전체 검증을 구분한다.
- Microsoft SimConnect 배포 권리를 확인한다. 현재 로컬 dist를 곧바로 공개 배포 가능하다고 표현하지 않는다.
- 공개 패키지는 별도 깨끗한 staging에서 만들고 pref/diagnostics/사용자 프로젝트를 제외한다. 기존 사용자 실행 폴더에서 설정을 삭제하거나 통째로 압축하지 않는다.
- 소스/바이너리 공개 여부·대상 GitHub 저장소·릴리스 업로드는 후속 지시에 따른다. 이번에는 로컬 커밋까지 수행한다.
