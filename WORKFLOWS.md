# 0.4.0 작업 및 릴리스 운영

## 현재 승인 범위 (2026-11-15)

사용자가 DCS 지원 버전을0.4.0으로 확정했다. 기존 문서뷰 개선은 사용자 확인 완료다. 버전/문서 정리, Settings Logging 숨김, 로컬 커밋, dist빌드, GitHub에 사용자가 올릴 Windows ZIP/SHA256 생성을 수행한다. Git태그/푸시/GitHubRelease생성/업로드는 요청되지 않았으므로 하지 않는다.

## 확정 구성

- DCS 자동 프로필 탐색과 Add/Update/Restore/Remove Scripts, 고정 Export 로더와 relay v2, 별도10–100ms설정(기본50ms), 로컬UDP43891/protocol1, 유효기간500ms.
- DCS기본5개: Ground Roll, Touchdown, Airframe Shake, G Load, Weapon Release. High AoA/Overspeed제외. Touchdown은센터gear.nose와좌우gear.main,기어별재무장/첫rod상승,Landing window없음.
- 사용자 확인: Ground Roll·P-51메인기어Touchdown, 자동탐색, Explorer정렬, 개선된README/매뉴얼표시. 센터수정후실기·무장효과·전기체/장치회귀는 미확인. 상세는docs/RELEASE-NOTES-0.4.0.md.
- 기본빌드는 Logging메뉴/수집코드제외. 개발진단이필요할때만 scripts/build.ps1 -Deploy -EnableTelemetryLogging. 이개발빌드는 package.ps1에서거부한다. 일반Debug Output/크래시진단은유지한다.

## 검증 및 패키징 순서

1. AGENTS/README/DEVELOPMENT/HANDOFF/WORKFLOWS와git status확인,기존작업보존.
2. scripts/build.ps1 -Deploy로버전리소스/C++/Lua구문검사와dist갱신. 앱/DCS/HID/모터/런타임테스트는사용자의별도요청없이실행하지않는다. 실행중앱을임의종료하지않는다.
3. 문서/버전/빌드설정/diff 및배포해시확인. 사용자dist의pref/프로젝트/진단은보존한다.
4. 승인된소스변경만선택해로컬커밋. DLL/.deps/build/dist/개인설정/진단은Git에추가하지않는다.
5. scripts/package.ps1은새 dist/packages/<고유ID>/StellaVibra staging에허용파일만복사하여StellaVibra-0.4.0-windows-x64.zip과.zip.sha256생성. 실제사용자실행폴더통째압축/삭제금지.
6. ZIP엔트리·필수파일·제외대상·SHA256확인. 앱실행없이패키지파일구조와내용일치만검증한다. HANDOFF갱신,새세션일지금지.

패키지에는 DCS relay/기본Lua,사용자영문문서/로고,동일빌드PDB,Qt/Lua런타임·대응소스·라이선스,원본SimConnect두버전/EULA/출처를포함한다. Microsoft재배포허가미확인고지는THIRD-PARTY-NOTICES와SIMCONNECT-LICENSING에유지한다. 공개작업절차는docs/PUBLISHING.md참조.

## 문서 및 상태

사용자문서는영어,내부기획/협업은한국어. 사용자README/매뉴얼/릴리스노트는현재0.4.0기능과일치시킨다. 이전결정과구현이력은HANDOFF/CONTRACTS의날짜별기록을참조하고최신계약을우선한다. docs/DCS-LOG-ANALYSIS.md는이전두비행로그의분석자료이며무장필드실기검증자료가아니다.
