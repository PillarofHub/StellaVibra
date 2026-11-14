# Stella Vibra 인수인계

## 최신 작업 — 2026-11-15 DCS 지원 0.4.0 배포 준비 완료

- 사용자가 개선된 README/매뉴얼 표시를 확인하고 DCS 지원 버전을 0.4.0으로 지정했다. 버전·문서·릴리스 노트를 갱신했으며 기존 문서 뷰 개선도 이번 커밋에 포함한다.
- 기본 build.ps1은 임시 Logging 메뉴와 수집 코드를 컴파일에서 제외한다. 개발 진단이 필요할 때만 -EnableTelemetryLogging을 사용한다. 이전 -WithoutTelemetryLogging 옵션은 제거했다. 일반 Debug Output/크래시 진단은 유지한다.
- package.ps1에서 누락됐던 DCS 중계 파일과 현재 릴리스 노트를 포함하도록 수정했다. Logging OFF, 현재 빌드와 dist 실행파일의 버전/해시, 소스 파일 해시를 검사한다. 사용자 실행 폴더 전체가 아니라 허용된 파일만 새 staging에서 압축한다.
- WORKFLOWS를 현재 배포 절차로 정리했다. 프로젝트/pref schema 1, DCS protocol 1, relay/manifest 2는 유지한다. 이전 기록의 0.2.1 및 미커밋 표기는 당시 상태이며 현재 상태는 이 항목을 따른다.
- 검증: scripts/build.ps1 -Deploy 성공(C++ 컴파일/링크, Lua 48개 구문 검사), ZIP 132개 파일을 staging과 해시 비교, SHA256 파일 확인. dist 개인 설정/프로젝트/진단 53개는 사전 해시와 일치한다. 앱/DCS/HID/모터 및 런타임 테스트는 실행하지 않았다.
- Qt 배포 도구는 translations/catalogs.json 및 선택적 dxcompiler.dll/dxil.dll 부재 경고를 출력했다. 빌드는 성공했으며 GUI 확인은 사용자 담당이다.
- 최종 업로드 대상: dist/packages/5cf40fcd01c24de8a47639dcf531aa01/StellaVibra-0.4.0-windows-x64.zip 및 같은 이름의 .zip.sha256. SHA256: a0a8989b7f2ba1fc4b8f627b70f53e2f5dab8c7f464ccead8c4c8def06f4a638.
- 요청한 변경은 로컬 커밋한다. 태그·푸시·GitHub Release·외부 업로드는 수행하지 않는다. 사용자가 직접 업로드한다. 패키징 중간 산출물보다 위 최종 경로를 사용한다.

## 최신 작업 — 2026-11-15 README·매뉴얼 표시 개선

- 시작 상태clean, 직전 변경은c968525에 이미 커밋됐다. 사용자의 '이 상태에서 커밋'은 기존커밋으로 충족하며 빈커밋 없음. 후속 표시 개선은 미커밋으로 남긴다.
- Qt6.8.3 소스 tar의qtextmarkdownimporter.cpp를 정적으로 확인: `<img ...>`가 HTMLtagdepth를 남겨 이미지와 이후일반본문을 accumulator에 잡아두는 경로. README를 `![Stella Vibra](assets/stella-vibra-logo.png)`로 바꿔 혼합HTML을 제거한다. 사용자매뉴얼에는 이HTML 블록이 없었다.
- DocumentationBrowser(QTextBrowser)로 기존상대링크·이미지·Back/Forward/Home 유지, 본문11pt·여백24·제목간격·고정폭code10pt·표padding7/border1·이미지폭최대720/가용폭100% 적용. 별도HTML 한벌이나WebEngine 추가없이 동일MD를GitHub와공유한다. GitHub와픽셀단위동일하지않음. 새GUI는 직접실행하지않고 사용자확인대상.
- 버전0.2.1 유지, 사용자프로젝트/pref/로그보존, 앱/DCS/HID/런타임테스트 없음. 커밋/태그/Release/푸시 없음.
- 검증: 최종 scripts/build.ps1 -Deploy 성공, C++컴파일/링크·Lua48개구문검사·diff --check 통과. 소스/Release/dist README·MANUAL·로고 및 Release/dist 실행파일/PDB 해시 일치. 사용자확인: README 로고/전체본문, 매뉴얼표/코드, 문서간상대링크 및뒤로가기, 좁은창이미지축소.

## 최신 작업 — 2026-11-15 Weapon Release·중단된 커밋 완료

- 이전 Landing window 제거·폴더 우선 정렬·로그 분석·상대 프로젝트 경로·센터 수정/출력배정 변경은 미커밋 상태로 보존되어 있었다. 사용자 후속 요청으로 함께 커밋한다. High AoA/Overspeed는 DCS기본에서 제거, weapons/weapon_release.lua 추가(최종5개). 사용자 프로젝트의 기존 효과는 건드리지 않는다.
- 공식 설치 Export.lua766–782행에Cannon.shells/Stations.count/container/weapon.level1..4 확인. relay v2에서 이 선택필드를 추가, station 숫자ID를문자열키로 보존(최대64), 노드상한1024/패킷16KiB 유지, payload초과면비행입력보존 위해생략. 고정loader/config/protocol1 유지. 사용자에게 Update Scripts 후DCS재시작 안내 필요. 실제설치조작 없음.
- Weapon Release Power40 Constant, Gun80ms/Release150ms, 출력둘다Stick:1. 기총 잔탄과 동일station/type/container 수량 감소를 구분해 짧은 요청. 첫표본/누락/세션교체는baseline, 증가/종류변경/신규station은500ms 억제. 연속감소는종료기한교체. 소멸을0으로 해석하지 않으므로 마지막방출 누락 가능, 재무장중같은종류제거 오검출은 완전구분 불가. 이전 두 비행로그에는 무장필드가 없어 실동작 검증자료가 아님.
- 배포 스크립트는 알려진 DCS두폐기템플릿만 Release/dist에서삭제, 사용자프로젝트·설정·진단보존. payload뷰/완성추가와센터rod완성경로정정. 버전0.2.1 유지, 태그/Release/푸시 없음.
- 검증: scripts/build.ps1 -Deploy 성공(C++컴파일/링크·Lua48개구문검사), diff --check 통과. 소스/Release/dist 템플릿·relay·README/MANUAL 및 실행파일/PDB 해시 일치, 두폐기템플릿 없음, 배포전후 사용자프로젝트/pref/진단 파일 해시 보존. 실제설치/GUI/게임/HID/효과런타임 테스트는 실행하지 않았다. 다음 사용자 확인은 relay Update Scripts+DCS재시작 후 Gun/Release 감소·재무장·제티슨·마지막1발의station표현이다.

## 최신 작업 — 2026-11-14 Landing window 제거·커밋

- 사용자 승인으로 DCS Touchdown Landing window ms, landing_until/previous_air 및 만료 분기 제거. 각 기어는 공중 재무장 후 비공중 첫 rod 상승을 독립적으로 기다리고1회 armed를 소모한다. 늦은 센터 접지는 시간 제한 없음. Rearm200ms/수치guard1e-6/첫 상승 구간 기한/첫 연결·누락·유효성 방지는 유지. 기어별 접지 신호 없이 rod를 쓰는 추정 한계는 MANUAL/CONTRACTS에 명시.
- 이전 미커밋 폴더 우선 정렬·로그 분석·센터 경로 수정/출력 재배정/High AoA·상대 New Project 경로까지 함께 선택해 사용자 요청 로컬 커밋. 버전0.2.1 유지, 태그/Release/푸시 없음. 사용자 프로젝트 복사본은 덮어쓰지 않으며 새기본Lua 복사 또는 새프로젝트 생성으로 적용한다. 제거된 Landing window/Minimum rod rise Override는 unresolved 정리가 필요할 수 있다.
- 검증: scripts/build.ps1 -Deploy 성공, 컴파일/링크 대상 최신·Lua49개 구문 검사·diff --check 확인. 게임/GUI/HID/모터/런타임 테스트 실행 없음. 늦은 센터 착지와 기어별1회 처리는 사용자 확인 대상.

## 최신 작업 — 2026-11-14 효과 보정·High AoA·상대 프로젝트 경로

- 이전 미커밋 폴더 우선 정렬/로그 분석 변경 보존. 센터 Touchdown은 gear.nose.rod로 수정. Ground Roll은 변경 없음. 사용자 언급 Landing window/Rearm/Minimum rod rise는 Touchdown의 설정이다.
- Rearm200ms와 Landing window1500ms 유지(공중 튐 재무장 억제/늦은 센터 감지 및 활주 중 오작동 제한). Minimum rod rise 노출 파라미터 제거,1e-6 내부 수치 guard. 기존 로그 양수 rod 증가의 호넷좌32/우41개는1e-6 이하, P-51은없음. 첫 상승 구간·첫 샘플/누락 억제 유지, 실행 검증 아님.
- Shake는 이미 소수→설정 범위 정규화→Power강도 반올림→C++0–255 검증→승자선택→HID바이트로 전달됨. 범위0–1은 설정 기본값이며 API 상한 보장이 아님. 기본 범위/Power는 유지, 출력만 Stick:1+Throttle:1/2. G Load는Throttle:1/2, Overspeed는Stick:1 유지.
- High AoA 새기본Lua(총6개): 공중·유효 AoA의 Start10→Full20 스케일, Power20 Constant, Stick:1, Warning enabled=false. 설치 공식 Export.lua454행rad표기와 로그(최대15.649/9.242)의 해석 불일치 때문에 단위를 임의 변환하지 않고 native 문턱으로 비교. 자동 실속/실제 버핏/조종석 AoA units 아님. 최종 체감/기체별 문턱은 사용자 확인.
- New Project에 폴더명/상대경로 입력 가능, portable launcher appRoot 기준(내부bin/processCWD 아님). Browse 초기경로도 일치. 정규화한 절대 경로로 덮어쓰기 확인·생성. 빈 입력 및 C:Projects 같은 모호한 drive-relative 거부.
- 기존 프로젝트 복사본/Override·로그는 보존. 새기본Lua를 기존프로젝트에 적용하려면 사용자가 복사/새프로젝트 생성 필요. 제거된 Minimum rod rise Override가 있으면 unresolved에서 정리한다. 버전0.2.1 유지, 커밋/태그/Release/푸시 없음.
- 검증: 최종 scripts/build.ps1 -Deploy 성공, C++ 컴파일/링크·Lua49개 구문 검사·diff --check 통과. Release/dist 실행파일/PDB/README/MANUAL·6개DCS기본Lua 및 소스 해시 일치, 분석 로그2개 해시 보존 확인. GUI/실제 상대경로 생성/센터착지/High AoA·HID 출력은 실행 검증하지 않았다.

## 최신 작업 — 2026-11-14 폴더 우선 정렬·P-51D/호넷 로그 분석

- 시작 git status clean. EffectSortModel로 디렉토리 여부를 먼저 비교하고 동일 종류는 기존 대소문자 무관 이름 비교를 유지한다. 파일 모델 재구성과 이름 변경 후에도 같은 프록시 비교 정책을 사용한다. README/MANUAL 갱신.
- 사용자 지정 로그2개를 f/d/x 복원 후 분석. P-51D 945표본, 호넷1788표본; 첫 기존 표본 포함으로 end samples보다1 많음. malformed/#dropped/구간 내 송신 순번 누락 없음. P-51 마지막에는 입력 만료1행. 상세31개 필드 범위/사건 시각/한계는 docs/DCS-LOG-ANALYSIS.md.
- Shake는0/1이 아님: P-51 최대0.2610·기본강도 최대8, 호넷 최대1.0443e-5·기본강도 전부0. 호넷 공중true는약0.33초라 충분한 비행 버핏/고G 검증자료 아님. 엔진RPM/플랩 변화는 확장 입력 후보, 무장/기총은 미수집.
- 센터 Touchdown 원인 확인: 원본은 gear.nose.rod, 코드가 gear.main.nose를 조회하여nil. P-51 착지104ms후, 호넷887ms후 nose 양수 관측. 이번 정렬/분석 단위에서는 효과를 수정하지 않았다. 다음 효과 수정 때 좌우는main, 센터는gear.nose로 읽어야 한다. 기존 조사 문서 경로는 정정했다.
- 버전0.2.1 유지, 이번 커밋 요청 없음. GUI/게임/HID/런타임 테스트 실행 없음. 원본 로그·사용자 프로젝트·설정 보존.
- 검증: scripts/build.ps1 -Deploy 성공, C++ 컴파일/링크·Lua48개 구문 검사·diff --check 통과. Release/dist 실행파일/PDB/README/MANUAL 해시 일치, 분석한 로그2개 해시 보존 확인. 폴더 우선 GUI 정렬은 사용자 확인 대상.

## 최신 작업 — 2026-11-14 실기 피드백·Explorer·텔레메트리 Logging

- 사용자 확인: DCS Ground Roll 양호, P-51 메인기어 Touchdown 정상. 센터(테일) 감지 미확인. Shake가0/1인지 아직 가설이며 원본 기록으로 검토한다. 기본 Shake/G Load/Overspeed Power.pwm=false, 기존 프로젝트 복사본·Override 보존.
- 새 프로젝트 Explorer 초기 비동기 로딩 전에 펼침 대기가 제거될 수 있는 경로 수정. 프로젝트 로드 시 새 파일 모델, directoryLoaded 후 루트/펼침 처리. 기존1초 Lua 동기화에 폴더/파일 경로 목록 비교 추가, 구조 변경 때 모델 갱신·선택/펼침 복원, 이름순 유지. 내용 편집·초안 보존 로직은 기존 경로 유지. GUI 확인은 사용자 담당.
- Settings → Logging 체크 토글. 후속 사용자 정정: Shake뿐 아니라 현재 수신하는 모든 텔레메트리/transport를 기록. 앱 시작 시 파일 생성 없음, 로깅 시작마다 diagnostics/telemetry/UTC_game_unique.jsonl 새 파일. 시작/종료UTC·게임·기체·세션 및 t/v/f/d/x 형식 전체+변경분(JSON Pointer), 누락/null/0 구분, 배열 통째 교체, 소수 보존, 끝 표본 수. 수신 루프에서 기록하며 UI 주기/출력 설정과 독립. 최대256KiB 큐→UI 저장,1초flush/정상종료flush, overflow #dropped 후 전체표본으로 재개, 쓰기 오류 중지. 프로젝트 전환/닫기·앱종료 중지, 시작off. 실제 로깅은 실행하지 않았음.
- 임시 기능 컴파일 스위치 STELLA_TELEMETRY_LOGGING 기본OFF. 로컬 build.ps1은ON, 공개 배포 준비 명령은 -WithoutTelemetryLogging(메뉴·수집 코드 제외). 이번 dist는 사용자 확인용ON으로 갱신한다. 공개 배포는 별도 요청이며 버전0.2.1 유지.
- 사용자가 이번 커밋을 요청했다. 이전 미커밋 기본 효과5개/MSFS 수신 수정과 이번 피드백/UI/로깅을 함께 선택해 커밋한다. 태그/Release/푸시 없음. GUI·게임·HID·런타임 테스트는 실행하지 않는다.
- 검증: 로깅 제외 build.ps1 -WithoutTelemetryLogging 컴파일·링크 성공, 전체 텔레메트리로 확장한 최종 build.ps1 -Deploy 성공(Logging ON). Lua48개 구문 검사 및 diff --check 통과. Release/dist 실행파일/PDB/README/MANUAL·DCS Lua 해시 일치, 기존 프로젝트·루트pref 해시 보존 확인. GUI/실제 로그 파일 생성·온오프/실게임은 사용자 확인 대상이다.

## 최신 작업 — 2026-11-14 DCS 기본 효과·MSFS 중앙 접지 회귀 수정

- 사용자 요청에 따라 대규모 사전 로그 수집 대신 기존 조사에 기반한 DCS 기본 Lua 5개를 구현했다. 실기/체감/기체별 반환은 사용자 검증이며, 자료의 존재를 모든 기체의 정상 동작으로 간주하지 않는다.
- effects/DCS 아래 Ground Roll, rod 첫 상승 Touchdown, Airframe Shake, G Load, 선택 Overspeed. 새 DCS 프로젝트에서 Generate default Lua scripts 활성. 기존 프로젝트는 변경하지 않았다. 기존 DCS 프로젝트에는 번들 effects/DCS의 원하는 Lua를 복사하거나 새 프로젝트로 시작할 수 있다.
- Ground Roll 전체3슬롯/strength1, Touchdown 기어별3슬롯/40, Shake Stick:1/30, G Load Stick:1/25(Positive2–6G), Overspeed Stick:1/20(Warning enabled 기본false). 정확한 파라미터/제약은 CONTRACTS 및 MANUAL 참조. rod 방향/갱신·Ny 축·shake 크기·세계속도 단위/항모 상대속도는 실기 확인 대상이다. 첫 샘플 가짜 착륙 방지, rod 누락 시 off, 통합 착지 대체 없음. High AoA/Thumps/별도 Stall/RPM·시동정지·무장은 이번 기본 묶음에 없음.
- dist SampleProject Ground Roll Lua는 MSFS2024 번들과 동일하며 중앙 기본 출력·개인 슬롯 배정도 있었다. 원인은 C++ SimConnect 고정 인덱스: on_any_runway 추가 후 contact[0]가 runway를, 나머지 접지/animation이 이전 필드를 읽었다. simVariables 이름 기반 값 참조로 수정(MSFS2020/2024 공통). SampleProject/개인 설정 편집 없음. 실게임에서 중앙 Ground Roll과 Touchdown을 다시 확인해야 한다.
- 검증: scripts/build.ps1 -Deploy exit0, C++ 컴파일/링크·48개 Lua 구문 검사 완료. Release/dist 실행파일/PDB/README/MANUAL 및 새5개 Lua 해시 일치. 배포 전후 SampleProject/DCSProject/루트 pref 파일 해시 보존 확인. diff --check 통과. 앱·게임·HID·모터·런타임 테스트는 실행하지 않았다.
- 버전0.2.1 유지. 이번 작업은 미커밋(이번 요청에 커밋 지시 없음). 이전 설치·수신 작업은1f6432d. 태그/Release/푸시 없음. 사용자 확인 시 효과 간 단일 승자 우선순위 때문에 하나씩 켜고 확인하면 원인 분리가 쉽다.

## 최신 작업 — 2026-11-14 config 숫자 편집 허용·빈 Export 삭제·커밋

- 사용자 요청: config.lua 숫자만 바꾼 경우 실제 값을 설정 창에 표시하고 해시 손상으로 처리하지 않는다. 파일 전체를 실행 없이 정해진 주석/return 형식으로 파싱하며 LF/CRLF·정수10–100ms 허용. 숫자 외 형식 변경/범위 오류는 Restore. config 값은 manifest의 저장 주기보다 우선한다.
- 무결성 해시는 loader.lua/telemetry.lua에만 적용한다. 이전 manifest의 config 해시는 무시하며 새 저장에서 제외한다. 확인 중 파일 변경 검출용 fingerprint에는 config도 포함한다.
- Remove에서 Stella 행을 제거한 결과가 정확히0바이트면 Export.lua 자체도 삭제. 공백/개행/BOM이 남으면 보존한다. 공유 파일 전체 복원은 하지 않는다.
- 검증: 최종 scripts/build.ps1 -Deploy exit0, C++ 컴파일·링크 및43개 Lua 구문 검사 통과. diff --check 및 Release/dist 런처·앱·PDB·README·MANUAL·DCS Lua 해시 일치 확인. GUI·실제 사용자 파일 설치/삭제/복구·앱/DCS/HID·런타임 테스트 없음. 버전0.2.1 유지.
- 이번 사용자 승인으로 누적 DCS 자동 탐색·설치/복구·수신·문서 변경을 한 로컬 커밋으로 정리한다. 태그/Release/푸시 없음. 사용자 확인 대상은 설정 숫자 수정 후 다시 열기, 범위/형식 오류 Restore, Remove 후0바이트 삭제와 다른 행 보존이다.
## 최신 작업 — 2026-11-14 DCS 로더 행 관리·복구 UI

- 사용자 요청 검토: 공유 Export.lua는 현재 Stella 관련 행만 관리하고 과거 전체 원본을 복원하지 않는 방식이 적합하다. 전체 Export 백업/원본 메타데이터/전체 복원과 새 백업 생성 제거. 과거 백업 폴더는 그대로 둔다.
- UI는 경로 → 주기 → Add/Update/Restore Scripts → Remove Scripts → Done. 로더 없음 Add, 정확한 한 행+관리 파일 해시 정상 Update(항상 활성), 수정/중복 행·손상/누락 파일 Restore. 복구는 상세 행 표시+기본No 확인. Remove는 해시 불일치여도 가능하며 전용 폴더 전체 삭제 범위를 확인한다.
- 현재 Export.lua에서 대소문자 무관 StellaVibra/Stella Vibra 포함 물리 행만 교체/제거한다. 복구 시 하나로 통합, 다른 행 바이트/BOM/개행 보존. 한 행에 섞인 다른 코드도 바뀐다는 점을 확인창에 명시한다. 임의 다중행 Lua 파싱은 하지 않는다.
- manifest schema2는 원본 Export를 저장하지 않는다. 이전 schema1은 해시/주기만 읽어 호환하며 다음 저장에서 구 원본 메타데이터를 제거한다. 확인 중 파일 변경은 fingerprint로 거부, 실패 롤백은 전용 관리 파일만 대상으로 한다. 공유 Export 전체 복원 없음.
- Remove는 canonical 경계/하위 링크 검사 후 전용 폴더 내용 전체 삭제. 다른 exporter·과거 백업은 보존. 실제 사용자 파일 설치/복구/삭제 및 앱·DCS·HID·테스트 실행 없음.
- 검증: 최종 scripts/build.ps1 -Deploy exit0. C++ 컴파일·링크 및 43개 Lua 구문 검사, diff --check 통과. Release/dist 런처·앱·PDB·README·MANUAL·DCS Lua 해시 일치. GUI/실제 설치·복구·삭제는 사용자 확인 전이다. 버전0.2.1 유지, 기존 미커밋 변경 보존. 커밋/태그/Release/푸시 없음. 사용자가 확인할 항목: 변형 로더/파일의 Restore 취소·승인, Add/Update 상태, 다른 exporter 행 보존, 손상 상태 Remove.
## 최신 작업 — 2026-11-14 DCS 실제 설치 및 프로젝트 수신

- 사용자 확인: 자동 탐색 정상. 이번 요청으로 UI 한정 범위를 넘어 실제 Add/Update/Remove와 DCS 프로젝트 수신을 구현한다. 효과 Lua는 아직 만들지 않는다.
- dcs/loader.lua·telemetry.lua 번들, 별도 config.lua 생성, manifest 기반 SHA-256 검사. Export.lua 정확한 한 줄만 추가하며 변경 전 고유 백업/파일별 QSaveFile 사용. 중복·수정·부분 설치는 거부한다. UTF16/32 편집도 거부한다. 제거는 관리 행/검증된 관리 파일만 대상으로 한다.
- Lua는 원본 콜백 체이닝/중복 로드 방지/오류 격리, 기본50ms(10–100ms) 프레임 제한 송신, nil 생략, ownship 권한 존중. 설정/코드는 DCS 재시작 후 반영한다.
- DCS Target Game 추가, 효과 없는 프로젝트 생성 가능. UDP127.0.0.1:43891 배타 바인딩은 해당 프로젝트 활성 중에만 수행한다. JSON 프로토콜1/최대16KiB, 세션·순번·송신 시각/500ms 유효기간 검증. 세션/객체ID/시간역행/공백에서 Lua 인스턴스와 출력 요청 초기화. 동일 스냅샷의 원래 수신 시각/기한 보존.
- Telemetry 뷰 및 자동완성은 telemetry.dcs 원본 경로와 transport 메타데이터를 제공한다. 무장/기총은 아직 수집하지 않는다. 기어 rod 의미/기체별 반환/실제 진동은 검증하지 않았다.
- 상세 파일·전송·원본 보존·실패 한계는 docs/DCS-INTEGRATION.md. README/MANUAL/CONTRACTS/WORKFLOWS/DEVELOPMENT 갱신.
- 검증: 최종 scripts/build.ps1 -Deploy exit0, 컴파일·링크 및 기존41개+신규2개 Lua 구문 검사 통과. 생성 config 형식도 LuaCompiler -p로 별도 구문 확인. Release/dist 런처·앱·PDB·DCS Lua·README·MANUAL 해시 일치, 소스/배포 Lua·문서 일치, 버전0.2.1 및 diff --check 확인. 첫 컴파일에서 다른 다이얼로그에 잘못 추가된 게임 선택 연결을 발견해 제거했다. 실제 사용자 Export.lua 설치/제거·앱/DCS/HID/런타임 테스트는 수행하지 않았다. 기존 미커밋 UI 변경 보존. 버전0.2.1 유지, 커밋·태그·Release·푸시 없음.
- 사용자 다음 확인: Add → DCS 재시작 → DCS 빈 프로젝트 생성 → Telemetry Live/원본 값·pause·임무 종료·기체 변경 확인. Update 주기/코드·Remove/백업 보존도 별도 확인. 기본 효과는 후속 단계다.
## 최신 작업 — 2026-11-14 DCS 자동 탐색·단순 UI

- 사용자 최신 요청/응답: 경로 선택 UI 제거, 자동 발견. 주기는 ms. 고정 중계 코드와 별도 주기 설정 파일을 StellaVibra Telemetry 아래 두는 방향에 동의했고 이번에는 UI까지만 진행하기로 했다.
- SHGetKnownFolderPath(FOLDERID_SavedGames)로 실제 Saved Games를 조회하고 직속 DCS/DCS.* 및 Config/options.lua·Logs/dcs.log를 함께 가진 사용자 지정 프로필 후보를 탐색한다. 하나면 자동 선택, 여러 개면 발견된 프로필 콤보만 표시한다. 미발견은 안내하며 임의 디렉토리를 생성하지 않는다.
- Browse/수기 경로/Save Selection/Refresh 버튼 삭제. 대상·상태·Update interval(50ms) 및 Add/Update Script → Remove Script → Done으로 단순화했다. dcs_scripts_path pref는 더 이상 참조/저장하지 않으며 기존 값은 삭제하지 않는다.
- 중계기/설치기가 아직 없으므로 주기 입력과 Add/Update/Remove는 비활성이다. 이름 흔적은 설치 인증/버전 판정이 아니며 현재는 버튼 문구만 변경한다. 다음 구현에서 버전/주기 변경 시 Update, 실제 설치 때만 Remove를 활성화해야 한다.
- DCS 공식 설치 예제에서 파일 자체가 아니라 콜백이 반복 호출됨을 확인했다. Export.lua 고정 로더, 별도 설정/콜백 체이닝 방향을 CONTRACTS에 기록했다. 포트/프로토콜/로더 파일명·API/주기 범위·적용 시점은 미확정이다.
- 검증: 최종 scripts/build.ps1 -Deploy exit0. C++ 컴파일·링크 및 기존41개 Lua 구문 검사, diff --check 통과. Release/dist 런처·앱·PDB·README·MANUAL 해시 일치. 이번 UI의 실제 자동 발견·GUI 동작은 사용자 확인 전이다. 실제 사용자 Export.lua 설치/삭제, 앱·DCS·HID·런타임 테스트 실행 없음. 버전0.2.1 유지, 커밋/푸시/태그/Release 없음.
## 최신 작업 — 2026-11-14 DCS 설정 메뉴 선행 구현

- 사용자 확인: f002a63의 이름 오름차순 정렬 정상. 최신 효과 제안 방향 승인(상세 DCS-TELEMETRY), 이번 요청은 메뉴/상태 검사만이며 중계기 자체는 만들지 않는다.
- Settings → Set DCS Telemetry... 추가. 프로필/Scripts Browse와 절대 Scripts 경로 입력, 실제 Export.lua 경로, 상태, 기본20 FPS/50ms, Add Script/Update Timing·Remove Script·Refresh Status·Save Selection/Cancel 제공.
- 앱 시작 시 저장 경로를 읽기 전용 검사해 로그 기록. 메뉴 열기/선택/편집 완료/Refresh에서도 재검사한다. 경로 미선택/폴더·파일 없음/읽기 실패/1MiB 초과/읽기 가능/이름 흔적을 구분한다. 자동 프로필 탐색이나 선택은 없다.
- 로더 계약이 없으므로 Stella 문자열은 힌트일 뿐 설치 인증 아님을 명시한다. 주석도 매칭 가능하며 흔적 발견 시 버튼 문구만 Update Timing으로 바뀐다. 설치·주기·제거 버튼 및 FPS는 모두 비활성. 중계기 구현 시 실제 설치 검사/활성화와 FPS 범위·반영 정책을 확정해야 한다.
- Save Selection은 앱 pref dcs_scripts_path만 저장한다. DCS 폴더/Export.lua/중계기/설정 생성·편집 없음. 실행 검증을 위해 사용자 Export.lua를 도구로 읽거나 수정하지 않았다.
- 검증: 중단된 빌드 세션은 종료되어 재실행했다. 최종 scripts/build.ps1 -Deploy exit0, C++ 컴파일·링크 및 기존 41개 Lua 구문 검사 통과. Release/dist 런처·앱·PDB·README·MANUAL 해시 일치, 앱 버전0.2.1 및 diff --check 확인. 앱·GUI·DCS·HID·런타임 테스트 없음. 버전0.2.1 유지, 새 커밋·태그·Release·푸시 없음. 다음 사용자 확인은 경로 선택/취소/저장 후 재시작 검사 로그와 상태 표시다.
## 최신 작업 — 2026-11-13 정렬 방식 교체 및 0.2.1 로컬 커밋

- 사용자 보고: 아래 이전 작업의 강제 재정렬로는 이름 오름차순 문제가 해결되지 않았다. 이전 기록을 GUI 검증 완료로 해석하지 않는다.
- 타이머와 내림차순→오름차순 강제 호출을 제거했다. 비동기 QFileSystemModel의 행 순서에 직접 의존하던 트리를 동적 QSortFilterProxyModel로 감싸 대소문자 구분 없는 이름 오름차순으로 표시한다. 폴더와 파일 모두 이름 기준이다.
- 프로젝트 루트/자동 펼침/생성·이동 후 선택에는 프록시 인덱스를 사용한다. 더블 클릭/메뉴/New File 기본 위치/드래그는 FilePathRole로 경로를 얻는다. 체크박스 setData는 프록시의 원본 모델 전달을 사용한다. 모델·탭 재생성 및 주기적 재정렬은 추가하지 않았다.
- 사용자 지정 버전 0.2.1로 CMake/README/MANUAL/내부 현재 기준을 갱신했다. 누적 Explorer 파일 관리와 DCS 조사 문서를 한 로컬 커밋에 포함한다. DCS는 여전히 조사/범위 논의 단계이며 지원 구현은 없다. 태그·Release·푸시 금지.
- 검증: 최종 scripts/build.ps1 -Deploy exit0, 컴파일·링크 및 41개 Lua 구문 검사 통과. Release/dist 런처·앱·PDB·README·MANUAL 해시 일치, 두 EXE 버전 0.2.1 확인. diff --check 통과. GUI·앱·게임·HID·런타임 테스트는 실행하지 않았다. 사용자 확인 필요: 폴더·스크립트를 z→a 및 a→z로 바꾼 직후 위치, 생성/드래그 후 순서, 체크박스·더블 클릭·드래그 대상 경로.
- 기존 개인 설정·프로젝트·진단을 보존하고 앱을 임의 종료하지 않는다.
## 최신 작업 — 2026-11-13 New File 루트 표시 및 Explorer 이름 정렬

- New File은 effects를 선택 가능한 최상위 폴더로 표시하고 그 아래 하위 폴더를 보여준다. 별도 Use effects root 버튼은 제거했다. 파일 시스템 모델의 프로젝트 직속 항목을 effects만 남기는 필터를 사용해 프로젝트의 다른 폴더는 보이지 않는다.
- 폴더 선택 시 effects/ 접두사를 포함해 경로를 채운다. 생성 시 수기 입력의 선두 effects/ 하나를 대소문자 무관하게 정규화하므로 effects/ground/test.lua와 ground/test.lua가 같은 파일을 가리킨다. 기존 이름·경계 검사와 중간 폴더 생성은 유지한다.
- Explorer는 이름 오름차순을 명시하고 헤더 클릭에 의한 변경을 막는다. 비동기 rowsInserted/rowsRemoved 및 이름 데이터 변경을 모아 큐에서 강제 재정렬해 rename 뒤 끝으로 밀린 항목을 되돌린다. 체크·오류 표시 갱신은 역할을 명시하여 불필요한 재정렬을 피한다. 모델/탭 초기화는 하지 않는다.
- README/MANUAL/CONTRACTS/WORKFLOWS 반영. 기존 모든 미커밋 변경 보존. DCS 설계/구현 범위와 버전은 변경하지 않았다. 최종 build.ps1 -Deploy exit0, 컴파일·링크/41개 Lua 구문 검사 및 diff --check 통과. Release/dist 런처·실제 앱·PDB·README·MANUAL 해시 일치 확인. GUI·앱·게임·HID·테스트는 실행하지 않았으며 기존 사용자 데이터 삭제/수정 및 커밋/푸시 없음.

## 최신 작업 — 2026-11-13 New File 경로·드래그 이동·삭제 보완

- 사용자 상시 지시: 앞으로 빌드는 -Deploy로 dist도 바로 갱신한다. 공개 배포/버전 확정은 별도다. AGENTS와 WORKFLOWS에 기록했고 이전 dist 미갱신 방침을 대체했다. 기존 설정/프로젝트/진단 보존, 앱 임의 종료 금지는 유지한다.
- File → New File은 effects 루트 폴더 트리, 상대 script path 입력, Use effects root 버튼을 제공한다. 폴더 클릭 시 파일명은 유지하고 폴더 접두 경로를 채운다. 경로 조각의 이름/예약명/이탈을 확인한 뒤 없는 하위 폴더를 만들고 주석 API 예제 Lua를 생성한다. 잘못된 입력은 대화상자를 유지하며 취소 시 생성하지 않는다. 폴더 생성 후 쓰기 실패는 빈 폴더를 남길 수 있다.
- 우클릭 New Folder/New Script는 기존 이름 입력 방식을 유지했다. untitled 자동 생성은 불필요한 파일/효과 자동 등록을 늘리므로 도입하지 않았다.
- Move To 메뉴 제거. EffectTree의 내부 drag/drop으로 Lua/하위 폴더를 effects 안의 폴더에 이동한다. QFileSystemModel의 직접 이동 대신 기존 UUID/설정/탭/초안 보존 로직을 호출한다. 외부 드롭, 파일 위 드롭, 루트/자기 하위/동일 대상/이름 충돌은 거부한다. 한 항목씩 이동한다.
- 휴지통 이동 실패 시 Permanently Delete 확인(기본 No)을 다시 표시하고 Yes에서만 영구 삭제한다. 실제 삭제 직전 effects 경계와 하위 링크/junction을 재검사한다. 부분 실패는 명시하고 파일 추적을 갱신한다. 이 기능의 실제 파일 삭제/휴지통 실행 검증은 하지 않았다.
- README/MANUAL/DEVELOPMENT/CONTRACTS를 현재 동작과 일치시켰다. DCS 계획/버전0.2.0은 그대로이며 DCS 구현은 진행하지 않았다.
- 검증: 최종 scripts/build.ps1 -Deploy exit0, C++ 컴파일·링크 및 기존 41개 Lua 구문 검사 통과. Release/dist 런처·실제 앱·PDB·README·MANUAL 해시 일치, diff --check 통과. 앱·GUI·DCS·HID·테스트 실행·커밋·푸시 없음. 기존 사용자 설정/프로젝트/진단과 Aurora는 변경하지 않았다.
- 사용자 확인: dist/StellaVibra/StellaVibra.exe → New File에서 폴더 클릭/루트 선택/새 중첩 경로/이름 충돌/취소 → 편집 초안과 기체 설정을 둔 Lua·폴더 드래그 → 휴지통 실패 환경에서 영구 삭제 No/Yes. GUI 동작은 사용자 확인 전이다.

## 최신 배포 — 2026-11-13 Explorer 파일 관리 사용자 확인용

사용자가 dist 빌드를 명시 요청하여 `.\scripts\build.ps1 -Deploy`를 실행했다(exit0). 최신 Explorer/File 메뉴 구현을 dist/StellaVibra에 배포했으며 Release/dist의 루트 런처·실제 앱·PDB 해시 일치를 확인했다. 기존 개인 설정·프로젝트·진단을 삭제하거나 덮어쓰는 작업은 하지 않았다. 앱 종료/실행·게임·HID·테스트 실행·커밋·푸시 없음. 사용자는 dist/StellaVibra/StellaVibra.exe에서 GUI를 확인할 예정이다. 아래 이전 기록의 dist 미갱신 상태는 이번 요청으로 대체된다. DCS 효과 자체는 아직 구현되지 않았다.

## 최신 작업 — 2026-11-13 DCS 효과 정리 및 Explorer 파일 관리

- 이전 문서 변경을 보존했다. 사용자 결정: MSFS와 억지 일치시키지 않는다. Ground Roll/Shake/G Load/선택적 Overspeed 유지, High AoA 및 Ground Thumps 기본 효과 제외. Touchdown은 rod 기반 첫 상승을 먼저 시도하고 사용자 실기 실패 시 통합 출력으로 단순화한다. DCS 코드/연동 구현은 아직 시작하지 않았다.
- 엔진 검토: P-51D/FW-190D9 FMOptions의 propBodyShakeK/propDisbalanceShakeLim과 공식 EFM head shake 설명을 확인했으나 LoGetShakeAmplitude까지의 통합 호출 경로/모든 엔진 성분 포함은 확인 못했다. TelemFFB 문서의 직접 engine-shake는 IL-2 지원과 구분했다. RPM 선형 합성을 넣지 않고 별도 RPM 효과 보류를 권고한다. 시동/정지·무장 후보의 최종 범위는 미확정이다. 상세는 docs/DCS-TELEMETRY.md 최신 절.
- UI 구현: src/window_files.cpp 신규, window.cpp/h 및 syncFiles 조정. effects 루트 New Folder/New Script, 하위 폴더에 Rename/Delete/Move To 추가, Lua에 Rename/Delete/Move To. File → New File을 Save 위에 배치했다. New File은 선택 effects 폴더 또는 루트에 Lua를 생성한다.
- 파일 이름/예약명/확장자 검증, effects 내부 경계 및 canonical 조상 검사, 링크/junction 거부, 덮어쓰기/폴더 합치기/자기 하위 이동 금지. 대소문자만 변경할 때 임시 이름을 거쳐 이동한다. raw JSON 초안 중에는 작업을 막고 모달 작업 동안 자동 파일 추적은 보류한다.
- 명시적 이동/이름 변경은 UUID·Global/Specific·편집 초안·열린 탭/undo를 유지하고 저장 경로를 갱신한다. 구조 저장 실패 시 파일명 복원을 시도한다(프로젝트 여러 파일 전체 원자 트랜잭션은 기존과 같이 아님). 삭제는 내용·설정·초안 제거 확인 후 QFile::moveToTrash, 실패 시 영구 삭제 없음. 파일 복원과 설정 복원은 별개다.
- 새 Lua는 init/update/self/Power/output 및 유효성/새 샘플/시간/log/vibrate 주석 예제를 제공한다. 출력 미배정이며 진동 예제 호출은 주석이다. 생성 파일은 즉시 등록·열기, 기존 효과는 변경하지 않는다.
- 사용자 README/MANUAL은 후속 development build 기능으로 표시했고 내부 DEVELOPMENT/CONTRACTS/WORKFLOWS를 갱신했다. 버전은 0.2.0 유지, 새 버전/릴리스는 미정.
- 검증: 최종 .\scripts\build.ps1(-Deploy 없음) exit0, C++ 컴파일·링크/기존 41개 Lua 구문 검사 통과. 실제 생성용 문자열을 build/new-script-example.lua로 추출해 LuaCompiler -p exit0. git diff --check 통과. 실행 테스트·GUI·DCS·HID/모터·프로브 없음. dist와 사용자 Export.lua/개인 설정·프로젝트·진단을 변경하지 않았고 앱 종료/커밋/푸시 없음.
- 사용자 GUI 확인: build/Release/StellaVibra.exe에서 별도 프로젝트로 메뉴 대상별 목록, 예제 생성/이름 충돌/취소, 수정된 스크립트 및 Global/Specific 유지한 폴더·파일 rename/move, 삭제 취소/휴지통, raw JSON 초안 차단을 확인한다. 사용 중인 dist는 이전 버전 그대로다.

## 최신 작업 — 2026-11-13 DCS 공통 입력 조사

- 사용자 정정: 후보 선택보다 제공 입력 조사가 먼저다. 호넷/머스탱/도라를 쓰지만 이 목록을 지원 범위의 기준으로 삼지 않는다. 하나의 DCS 프로젝트/공통 요소를 목표로 한다. 아래 이전 기록의 우선 기체 선택/먼저 범위 합의 요청은 대체되었다.
- 시작 시 이전 작업의 CONTRACTS/HANDOFF/WORKFLOWS 변경 3개를 보존했다. 코드/사용자 데이터 변경 없음.
- 읽기 전용으로 설치 `D:\Eagle Dynamics\DCS World`를 발견했고 autoupdate.cfg에서 2.9.31.29434 확인. Scripts/Export.lua, API/Sim_ControlAPI.md, FM 헤더/예제, 설치 모듈 FM Lua, wsTypes를 조사했다. 공식 ED 변경 기록과 TelemFFB exporter도 대조했다.
- 상세 결과는 docs/DCS-TELEMETRY.md(설계 조사 문서, 세션 일지 아님). API/필드/단위·불확실성·효과 가능성·수집 비용/주기·pause/멀티플레이를 기록했다.
- 중요 정정: LoGetInAir와 LoGetShakeAmplitude가 공식 참조에 있고 LoGetMechInfo.gear.main의 rod 필드도 있다. 다만 rod 단위/방향/전 기체 갱신은 확인되지 않아 첫 압축 상승 Touchdown은 아직 확정 불가. 외형 인자1/6/4는 사용 사례가 있지만 범용 보장은 아니다. EFM suspension 구조체는 Export 입력이 아니다.
- 공통 후보: 통합 Ground Roll, Airframe Shake, High AoA, G Load, Airspeed, Engine RPM. Touchdown 통합 고정 펄스, 시동/정지 RPM 추정, 무장 잔량 감소 효과는 의미를 낮추는 타협안이며 미합의다. 무장 종류는 wsTypes로 분류 가능하지만 발사/비상투하/일부 재무장 구분을 해결하지 않는다. 포장/활주로 전용 Ground Thumps의 필수 공통 입력은 찾지 못했다.
- 다음에는 조사 결과를 근거로 공통 효과 범위와 타협점을 논의한다. 사용자에게 기체 목록이나 미확인 입력을 전제로 선택을 다시 요구하지 않는다. 포트/프로토콜/설치/UI/효과 구현은 아직 시작하지 않는다.
- 검증: 내부 문서 diff/참조 점검만 수행. 앱·DCS·HID·테스트·빌드·배포·프로브 실행 없음. Saved Games/사용자 Export.lua 및 dist, Aurora 변경 없음. 버전/커밋/푸시/태그/Release/업로드 없음. 문서상 확인과 런타임 검증을 구분한다.

## 최신 작업 — 2026-11-13 DCS 1단계 제안

- 시작 시 git status --short 출력 없음(깨끗한 작업 트리). AGENTS/README/docs/DEVELOPMENT/CONTRACTS/HANDOFF/WORKFLOWS 및 MSFS 지상 효과 Lua를 확인했다. 기준 0.2.0 유지.
- 사용자 요청은 단계별 논의다. 현재는 효과 후보 제안이며 2–5단계 구현 승인으로 해석하지 않는다. CONTRACTS/WORKFLOWS에 작업 경계와 미확정을 기록했다.
- MSFS Ground Roll은 Power1/PWM20/80ms, 최소0.1knots, 100ms 갱신 요청이다. Touchdown은 Power40/Constant, 최소200ms/최대2000ms/배수2/최소5knots, 공중 관측 후 재무장과 바퀴별 첫 압축 상승 구간을 사용한다. Ground Thumps는 실제 이음매 센서가 아닌 접지·속도·노면 조건의 가상 슬랩 모델이다.
- 제안 후보는 지상3종, High AoA·Stall/Buffet·G Load·Airspeed, 엔진 RPM·Startup·Shutdown, 무장 투하/발사·Gunfire다. 입력 보장 전에는 제공 효과로 확정하지 않는다. 접지만 있고 압축이 없을 때 고정 Touchdown 펄스를 허용할지, 기체 전체 접지의 통합 출력을 허용할지, RPM/무장 수량 변화 추정을 어느 범위까지 허용할지, 우선 지원 기체를 사용자와 결정해야 한다.
- 외부 예비 조회: DCS-ExportScripts Su-25T 원본의 LoGetEngineInfo/LoGetMechInfo 사용 사례를 확인했다. 주석 속 API 목록은 현행 지원 확인으로 취급하지 않았다. 공식/로컬 설치 자료 및 실제 신호·단위·전 기체 가용성 확인은 2단계에 남겼다. 출처는 CONTRACTS에 기록했다.
- 권장 범위: Ground Roll/Touchdown 우선, High AoA/G Load/Airspeed/Engine RPM은 입력 확인 조건부, 시동·정지·무장은 구분 근거 확인 조건부, Ground Thumps/직접 버핏은 필수 신호 확인 전 보류. 이는 합의 전 제안이다.
- 다음 응답에서는 사용자 선택을 반영해 1단계 범위를 먼저 확정한다. 입력 조사에서 조건을 충족하지 못하면 축소/대체안을 다시 논의한다. 포트·로더·FPS 범위·갱신 정책·버전은 아직 미정이다.
- 이번 변경은 내부 문서 3개뿐이다. 제품 기능 미변경으로 README는 유지한다. 앱·DCS·HID·테스트·빌드·배포·커밋·푸시·태그·업로드 없음. dist 및 사용자 Export.lua, Aurora는 건드리지 않았다. 정적 검증은 문서 diff 점검이다.

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

## 2026-11-11 — UI 확정 커밋 / 게임별 연결 확장

선행 누적 작업을 사용자 요청에 따라 세 커밋으로 기록했다: d95db7e(Lua 분리·카테고리 효과), 2d08eb0(편집기 UI·레이아웃·체크박스·진단), eb08984(Lua 구문 컴파일·문서). 푸시하지 않았다. 이번 신규 구현은 미커밋 작업 트리에 남긴다.

구현 내용:
- Project.game에 MSFS2024 / MSFS2020 / XPlane12 / WarThunder 허용. 프로젝트가 선택한 연결만 활성화하며 close/게임 전환은 연결·샘플·출력 요청을 정리한다. 게임을 모르면 다른 게임으로 대체하지 않는다.
- MSFS58개 숫자/bool+TITLE: 기존48개에 surface_type, g_force, acceleration_body_xyz, incidence_alpha, stall_alpha, gear_total_pct_extended, brake_left/right_position 추가. MSFS2020 공식 SDK0.24.6 DLL 별도 배치. 로컬 MSFS 두 버전 동시 실행은 기본 파이프가 모호하므로 연결 보류하며 로그로 안내한다.
- X-Plane12: Qt UDP localhost49000 RREF, Aurora의85개 카탈로그. 빠른 값20Hz/느린 값1Hz, 배치 재구독과 해제, 필드별 만료, network_time_sec/pause/acf_descrip 입력 유효성.
- WT: Qt 비동기 HTTP localhost8111 /state+/indicators, 최대20Hz, 350ms 제한/실패1초 재시도. 원본 JSON 전체 키를 그대로 Lua에 전달. 신뢰할 게임 clock이 없으므로 값 변화 없는500ms는 보수적으로 출력 중지. 새 HTTP 응답과 호스트 재전달을 구분한다.
- 게임별 Telemetry 뷰·원본 키 자동완성. XP `telemetry["`, WT `telemetry.state["`/`.indicators["`에서 완성. WT 실제 수신 키도 추가. 게임 사이 단위/필드를 공통 telemetry로 변환하지 않는다. C++ 내부 프레임은 실행 관리용 메타데이터만 공유한다.
- 배포 templates는 effects/<game>/...로 분리. 새 프로젝트는 선택 게임의 하위 파일만 effects/...로 복사. MSFS2024/2020 각11개, XP11개(RPM 기반 엔진 진동), WT8개(AoA cue·수동 과속·플랩/기어 이동·무장·엔진3). WT에 확인되지 않은 바퀴 접촉/압축을 만들어 Ground Roll/Touchdown으로 보고하지 않는다. XP Touchdown은 타이어 수직 압축의 첫 상승만 측정한다.
- New Project: 없는 절대 폴더는 생성. 기존 폴더는 기본 No 확인 후 프로젝트 파일·이름이 겹치는 기본 스크립트만 QSaveFile로 교체. 나머지 파일과 aircrafts는 보존하고 남은 Lua도 자동 발견한다. 다중 파일 트랜잭션은 아니다.

최종 검증: `.\scripts\build.ps1 -Deploy` exit0. C++ 컴파일·링크 및 전체41개 Lua `luac -p` 통과. 최종 증분 빌드의 프로젝트 컴파일/링크 진단0개, `git diff --check` 통과. SDK 헤더의 기존 signed→DWORD 경고는 전체 재빌드 시 나올 수 있고 번역 제외 windeployqt 경고는 런타임 누락을 뜻하지 않는다. 앱/게임/HID/모터/런타임 테스트는 실행하지 않았다. Release/dist의41개 템플릿 해시·Qt6Network/qwindows 존재·2020 DLL 해시를 확인했다. 두 EXE SHA-256은 동일한 `145D65F7C67E03C2351600274671F07694A398625EE629E6EF795AB807179A62`다. 실행 경로는 `E:\Workspace\Stella Vibra\build\Release\StellaVibra.exe`, 포터블 경로는 `E:\Workspace\Stella Vibra\dist\StellaVibra`다.

사용자 수동 확인:
1. 없는 경로에 네 게임 각각 새 프로젝트를 만들고 생성된 폴더·스크립트와 게임별 자동완성을 확인한다. 기존 폴더는 No일 때 변경 없음, Yes일 때 안내한 파일만 교체됨을 확인한다.
2. Stop All 상태로 대상 게임 하나를 시작해 연결 상태와 native 필드·단위를 확인한다. XP localhost UDP49000 및 WT localhost HTTP8111을 사용하는 기본 설정이 필요하다.
3. Resume 후 한 효과·낮은 세기로 확인한다. 게임 종료/정지/기체 변경/프로젝트 전환/Stop에서 해제되는지 확인한다. XP 타이어/엔진 dataref, WT 무장 번호·AoA·속도 한계·power/RPM 제공 여부는 기체별로 다르다.
4. WT는 모든 값이 일정하면 실제 비행 중에도500ms 뒤 멈출 수 있다. 공식 clock/pause 부재에 따른 보수적 제한이다. XP RREF는 원자 프레임이 아니라 개별 필드 갱신이며, 커스텀 애드온의 기본 dataref 갱신은 보장하지 않는다.

공식 SDK2020 원본/해시·배포 라이선스 제한은 SIMCONNECT-LICENSING.md에 기록했다. 두 버전의 DLL 동봉은 로컬 개발용이며 공개 재배포 권리 미확인 상태는 그대로다.

## 2026-11-11 — 실행 폴더별 포터블 설정

사용자 요청에 따라 앱 설정 위치를 `%LOCALAPPDATA%\StellaVibra\StellaVibra\local.json`에서 `StellaVibra.exe` 옆 `StellaVibra.pref`로 변경했다. 확장자는 pref이며 기존 UTF-8 JSON version1 스키마와 QSaveFile 원자 저장을 유지한다. 창 크기/위치·도킹 뷰 위치/표시·last_project/recent_projects·모터 매핑·tick_ms가 모두 같은 경로를 사용한다. applicationDirPath 기준이므로 바로가기 시작 위치와 무관하고 실행 폴더별로 분리된다.

기존 AppData 파일은 읽기/자동 이전/삭제하지 않았다. 새 위치에 pref가 없으면 기본값으로 시작하고 첫 설정 저장 시 생성한다. 이전 설정 유지가 필요하면 앱 종료 후 기존 local.json을 실행 폴더의 StellaVibra.pref로 수동 복사한다. 종료 후 pref 삭제로 앱 옵션을 초기화할 수 있다. 실행 폴더 전체 삭제 시 앱 옵션도 삭제되지만 외부 프로젝트 데이터는 보존된다. 실행 폴더 쓰기 권한이 없으면 오류를 표시하며 AppData/레지스트리 우회 저장은 없다. 배포 스크립트는 사용자 pref를 생성하거나 덮어쓰지 않는다. 설정 파일은 Git에서 제외한다.

검증: `.\scripts\build.ps1 -Deploy` exit0, C++ 컴파일·링크 및 Lua 구문 컴파일 통과. `git diff --check` 통과. src의 QSettings/AppLocalDataLocation/QStandardPaths/local.json 참조가 없는 것을 확인했다. Release/dist EXE 해시 일치. 앱·게임·HID·런타임 테스트는 실행하지 않았다. 기존 미커밋 게임 확장 작업 보존, 커밋·푸시 없음.

사용자 확인: 뷰 배치·틱 변경 후 종료 → EXE 옆 pref 생성 → 재실행 복원. 다른 실행 폴더에서는 pref 없이 기본값 시작. 종료 후 pref 삭제 → 마지막 프로젝트·배치·매핑·틱 초기화. 쓰기 불가능한 폴더는 설정 저장 오류가 발생하므로 쓰기 가능한 폴더에서 사용한다.

## 2026-11-11 — Set Timings

Settings → Set Tick을 Set Timings로 변경하고 기존 설명문을 제거했다. HID tick(5..100ms, 기본20), Maximum update period(5..500ms, 기본100), Reset to Defaults, OK/Cancel을 제공한다. 기본값 복원도 OK를 눌러야 적용된다. pref의 tick_ms와 maximum_update_period_ms에 저장·복원하며 새 키가 없으면100ms다.

엔진의 고정100ms 최신 스냅샷 재처리 조건을 atomic 설정값으로 대체했다. 새 텔레메트리는 계속 즉시 처리한다. 최대 간격만 바꿀 때 상태/요청은 유지하며 HID tick 변경의 기존 상태 초기화는 유지한다. 수신 시각·입력 만료500ms·heartbeat250ms·게임 구독·Telemetry 뷰 갱신 간격을 바꾸지 않는다. 실제 호출 간격은 OS/실행 루프 지연의 영향을 받는다.

사용자 확인: 두 값 변경 후 OK → 재실행 값 복원, Reset to Defaults →20/100, Cancel → 기존 값 유지. GUI·게임·HID·런타임 테스트는 실행하지 않는다. 기존 작업 보존, 커밋·푸시 없음.

검증 결과: .\scripts\build.ps1 -Deploy exit0. C++ 컴파일·링크와 Lua 구문 컴파일 통과, git diff --check 통과, Release/dist EXE 해시 일치. 실행 파일은 build/Release/StellaVibra.exe 및 dist/StellaVibra/StellaVibra.exe에 갱신했다. 앱은 실행하지 않았다.

## 2026-11-11 — Lua 출력 기본 논리 슬롯

이전 구현은 New Project의 C++에서 출력 이름(Airframe/Left Gear/Right Gear/Engine N)에 따라 기본 목적지를 생성해 프로젝트 Global outputs에 저장했다. Center Gear는 미배정이었다. 이 규칙을 제거하고 init의 `context:output(name, default_slots)` 등록에 기본값을 옮겼다. 예: `context:output("Center Gear", {"Stick:1"})`. 생략/nil/빈 배열은 미배정. 복수 슬롯 가능, 슬롯 이름과 연속 배열·중복·최대20개를 검증한다.

LuaEffect.outputDefaults를 configure·UI cacheDefaults·최소 저장·리셋·출력 선택창에 반영했다. Script Default→Global→Specific 순서이며 명시적 빈 배열은 상속을 차단한다. 기본값과 같은 Global은 저장에서 제거한다. 새 프로젝트는 outputs 기본값을 복사하지 않으며 이름에 따른 C++ 추측도 없다. 기존 프로젝트 소스/사용자 설정을 자동으로 변경하지 않는다.

모든 게임의41개 기본 Lua에서 output 등록 반복문을 풀었다. Center Gear=Stick:1, Left/Right Gear=Throttle:1/2, Airframe=Stick:1, Engine1..4=Throttle:1..4를 각각 명시한다. 엔진 상태는 명시적 테이블로 초기화하고 update의 출력 처리 반복문 및 Touchdown의 상태 초기화 반복문은 유지한다. 첫 압축 상승 판정 등 게임 효과 알고리즘은 변경하지 않는다.

사용자 확인: 새 프로젝트에서 Center/Left/Right 채널 기본값 확인 → Global 변경/Reset to Default → Specific 변경/Reset to Global → 빈 목록 선택 후 저장·재열기로 미배정 유지 → 기본값과 같으면 sv outputs 항목 생략 → default_slots 생략한 별도 출력은 미배정. 앱·게임·HID·Lua 콜백 테스트는 실행하지 않는다. 기존 미커밋 변경 보존, 커밋·푸시 없음.

검증 결과: .\scripts\build.ps1 -Deploy exit0. C++ 컴파일·링크 및41개 Lua luac -p 통과. 프로젝트 컴파일 진단0개, git diff --check 통과. Release/dist EXE 및 모든41개 Lua 파일의 소스 대비 해시 일치. 앱·런타임 테스트는 실행하지 않았다.

## 2026-11-11 — Browse 새 폴더 생성 / MSFS 시작 시 크래시 조사

Browse 수정: Qt6.8.3 qfiledialog.cpp를 확인했으며 Directory 모드는 없는 경로를 거부하고 선택 버튼도 비활성화한다. New Project Browse를 QFileSystemModel/QTreeView+경로 입력의 폴더 전용 다이얼로그로 변경했다. Select Folder에서 상대 이름은 선택 폴더 기준으로 해석하고 없는 중간 폴더까지 mkpath로 자동 생성한다. 기존 빈 폴더는 New Project의 경고 없이 사용한다. Hidden/System/하위 항목이 하나라도 있는 폴더만 기본 No 덮어쓰기 확인을 유지한다. Browse 취소는 생성하지 않고, 선택 완료 후 New Project 취소는 이미 생성한 폴더를 남긴다. Windows 경로 구분자를 정규화했다. 최초 build.ps1 -Deploy exit0 및 diff/Release-dist 해시 일치를 확인했다.

사용자 크래시 보고: dist/StellaVibra에서 앱을 켠 뒤 MSFS를 실행했고 다른 조작을 하지 않았다. Windows Application 이벤트1000/1001을 읽어 2026-11-11 17:15:11(KST) 해당 EXE의 0xc0000005 접근 위반을 확인했다. 오류 모듈 unknown, WER BEX64/StackHash_c4dc, ReportId84c62189-35fa-4c10-a4f7-5f9473968e5c. 로드 모듈에 배포 SimConnect.dll이 있다. ReportArchive에 Report.wer만 남아 있고 이벤트가 가리킨 임시 .dmp는 존재하지 않는다. 기존 Debug Output은 메모리 전용이었고 빌드 로그는 실행 로그가 아니다. 과거 새벽의 Qt/ucrt 크래시와 이번 이벤트를 동일 원인으로 단정하지 않는다.

정적 검토에서 연결 실패/재시도마다 SDK DLL을 FreeLibrary하는 수명 관리 위험을 발견했다. 연결 핸들은 기존처럼 Close하되 GetModuleHandleEx(FROM_ADDRESS|PIN)으로 실제 로드한 모듈을 프로세스 종료까지 유지하도록 보완했다. Windows 공식 PIN 의미를 확인했지만 이 크래시가 SDK 비동기 콜백/언로드 때문이라는 확증은 없고, 해결 검증도 하지 않았다. 첫 텔레메트리 디코딩·HID 출력에서 이번 주소0 접근 위반을 직접 설명하는 근거도 아직 없다.

추가 진단: 일반 실행에서 EXE 옆 diagnostics/<UTC시각>-PID.log에 Qt 메시지/Debug Output, SimConnect Open/결과 HRESULT, 첫 텔레메트리·첫 Lua update·첫 HID 출력 단계를 기록한다. 매 메시지 flush,2MiB 초과 시 .previous 하나로 회전한다. 레지스트리/AppData 설정 변경이나 자동 업로드는 없다. 처리되지 않은 예외에서 같은 세션명.dmp 생성 시도(MiniDumpNormal+ThreadInfo+UnloadedModules), 원래 Windows 예외 처리는 계속한다. 프로세스 내부 수집이므로 데드락/강제종료/fail-fast/권한 문제 등 모든 실패를 보장하지 않으며 고의 크래시 테스트는 하지 않는다. /Zi 및 /DEBUG로 Release PDB를 생성·동봉해 다음 덤프의 소스 분석을 준비한다.

사용자에게 요청할 확인: 새 dist EXE 실행 → Stop All → MSFS 실행. 이 상태에서도 죽으면 Lua/게임 출력보다 연결 경로를 우선 조사한다. 안정적이면 Resume 후 재발 여부를 확인한다. 재발 시 diagnostics의 해당 .log(.previous 포함)/.dmp 및 당시 StellaVibra.exe/.pdb를 업데이트 전에 보존하고 마지막 동작/시각을 알려준다. 덤프가 없더라도 로그를 남긴다. 기존 사용자 프로젝트·pref는 유지한다. GUI·MSFS·HID·모터 실행은 하지 않았다. 최종 빌드 결과는 아래에 기록한다.

최종 검증: build.ps1 -Deploy exit0. C++ 컴파일·링크 및41개 Lua 구문 컴파일 통과. 프로젝트 코드 진단0개, git diff --check 통과. Release/dist의 EXE와 새 PDB 해시 일치. 앱·MSFS·HID·덤프 생성 테스트는 실행하지 않았다. 기존 미커밋 변경 보존, 커밋·푸시 없음.

## 2026-11-11 — 창 크기 변경 시 뷰 비율 유지

사용자 확인: 이전 배포본에서 MSFS 2024 연결 및 이착륙 시 기대한 진동이 정상 동작했다. 이는 사용자 실기 확인이며, 과거 크래시의 정확한 원인을 덤프로 확정한 것은 아니다.

창이 안정된 시점의 도크 폭/높이 비율을 기억하고 전체 창 resize 후 resizeDocks로 적용한다. 수동 분할선 조절·도크 이동·숨김/표시 이후에는 새 비율을 기억하며 부동 창과 화면 밖 비활성 탭은 제외한다. 자동 크기 재분배 중에는 비율을 다시 수집하지 않아 축소/확대 반복 시 반올림·최소 크기 제약이 누적되는 것을 막는다. 도킹 애니메이션은 꺼서 지연된 배치 변경이 비율 수집과 충돌하지 않게 했다. Reset Default Layout은 대기 중인 이전 비율 적용을 취소한다.

기본 본문/Debug Output은 3:1, 좌우 1:4:1 및 Parameters/Script 1:1은 유지한다. 도크 최소 크기 힌트, 콘텐츠 크기 정책·레이아웃 제약, 내부 분할 패널과 상태표시줄의 최소 폭 제약을 완화했다. 내부 QSplitter는 동일 stretch와 최소 크기 완화로 사용자 비율을 따라 크기를 변경한다. 제목줄/분할선/픽셀 단위 제약은 남고, 아주 작게 줄인 내용은 잘릴 수 있다. layout_version4와 기존 저장 배치를 보존하므로 기존 사용자에게 새 기본 비율을 강제하지 않는다. 새 기본값은 View → Reset Default Layout으로 적용한다. 시작 시 restoreState를 반복 호출하지 않는다.

수동 확인: Reset Default Layout → 창 축소/확대·최대화/복원 → 각 분할선을 임의 비율로 조절 후 다시 창 크기 변경 → Parameters/Script도 비율 유지 확인 → 도크 숨김/표시·부동 분리·재도킹 → 종료/재시작 후 배치 복원. 앱·게임·HID 실행 및 테스트 실행은 하지 않는다. 기존 미커밋 변경 보존, 커밋·푸시 없음.

검증 결과: .\scripts\build.ps1 -Deploy exit0. C++ 컴파일·링크 성공, 컴파일 진단0개. git diff --check 통과. Release/dist EXE 및 PDB 해시 일치. 실행·GUI 리사이즈 동작 검증은 사용자 확인으로 남긴다.

## 2026-11-12 — 기본 Lua 조정, 채널 수, 배포 정리

중단된 작업을 이어 완료한다. MSFS2024/2020 기본11개, XPlane12 11개, WT8개(전체41개)를 대상에 맞춰 수정했다. 일반 갱신 요청150→100ms, Ground Roll strength1/PWM ON/minimum0.1knots/step0.1(X-Plane은 Lua에서 원본m/s 변환), Touchdown minimum200/maximum2000/multiplier2. 그 외 기본 PWM OFF. MSFS/X-Plane Overspeed 수동 한계 제거; WT는 게임 한계 입력이 없어 수동 한계 유지. Shutdown/Thrust/RPM/Power는 haptic_power를 복사하고 strength만 변경해 그대로 전달하며 PWM을 다시 구성하지 않는다. 시동/감쇠/착륙 이벤트 자체의 지속 시간 계산은 유지한다.

Ground Thumps: Power, Slab length(4.6m), Minimum speed, Maximum speed, Pulse ms(100), Ground type만 등록.3개 기어의 접지별 거리 적분으로 가상 경계를 통과할 때만 요청하며 첫 샘플/시간 역행/큰 간격/슬랩 변경을 재설정한다. 놓친 경계는 몰아서 재생하지 않는다. 펄스는 예상 경계 간격의 절반 이하, 재요청은 적어도 같은 길이의 휴지 후에만 허용한다. 강도 합산은 없지만 다른 효과가 계속 선택되면 전체 모터 출력은 이어질 수 있고, HID tick보다 짧은 펄스는 구분 보장 불가. 실제 휠베이스·이음매 위치가 없는 근사 모델이다. FAA Table3-7의15ft(4.6m)를 예제 간격으로 참고했다. MSFS on_any_runway Bool을 추가하여 활주로+surface_type(Concrete0/Asphalt4) 조건 사용. X-Plane 공식 surface_texture_type(Concrete2/Asphalt3) 추가, 유도로/계류장 구분 불가. WT 지상 입력을 추정해 추가하지 않았다. 카탈로그는MSFS59+TITLE, XP86.

Set Channels: Yoke/Stick/Throttle/Rudder0..4, 기본4; pref channels 키는 Yoke/Stick/Throttle/Pedal. 기존Wheel4 유지. 기본 출력만0이면비움/초과면(번호-1)%개수+1/중복제거. UI 캐시·리셋·최소저장·runtime configure에 동일하게 적용한다. 엔진 Bundle에 counts를 전달하고 counts 변경도 설정 변경으로 인식해 재적용한다. 명시적 프로젝트/기체/물리 매핑은 그대로 유지한다. 노면 선택용 enum 파라미터(문자열default/choices1..64개 연속·중복없음) 추가, 문자열 저장과 드롭다운을 지원한다. 숫자는 QDoubleSpinBox textFromValue에서 로캘 소수점 뒤0만 잘라 값/입력 정밀도를 보존한다. 파라미터 Form은 상단정렬, 각 행은 세로Fixed로 제한한다.

배포: 루트 StellaVibra.exe는 정적CRT Win32 런처이며 bin/StellaVibra.exe를 실행한다. dumpbin에서 런처 의존성이 KERNEL32/USER32뿐임을 확인했다. Qt/MSVC DLL=bin, Qt plugins=bin/plugins+qt.conf, SimConnect=bin/simconnect/MSFS2024 또는MSFS2020. 문서=docs, PDB=bin, 라이선스=licenses, 소스=sources, 템플릿=effects, 진단=diagnostics. pref와 템플릿/고지는 실제 앱의 부모 배포루트 기준이다. 루트/실제 앱 모두 같은 폴더 옵션을 사용한다. build Release의 LuaCompiler/lib는 tools/lib로 분리했다. 초기 배포는 이전 생성파일 정리의 Remove-Item 오류로 중단되었으나 -Force 및 정확한 오류 위치 표시를 적용한 뒤 정상 완료했다. 사용자 pref/프로젝트 및 알 수 없는 파일은 정리 대상에서 제외했다. SimConnect 공개재배포 권리 미확인 결론은 그대로다.

기존 사용자 프로젝트 Lua/Global/Specific을 자동 덮어쓰지 않았다. 새 프로젝트로 확인하거나 필요한 Lua만 직접 복사한 뒤 예전 Override를 Reset to Default/Global로 정리해야 새 기본값이 보인다. 수동 검증: 루트 EXE 실행→기존pref 복원→새MSFS2024 프로젝트→숫자/위쪽정렬/Power·Touchdown 기본값→Set Channels에서Stick0/Throttle2로 미배정·순환 확인→리셋/저장/재열기→노면드롭다운→실기에서 활주로/유도로/노면·속도 범위·개별기어 Thumps 및 타효과 우선순위 확인. X-Plane/WT도 해당게임 실기 확인이 필요하다. 앱·게임·HID·Lua 콜백·런처 실행 테스트는 하지 않았다. 커밋·푸시 없음.

최종 검증: .\scripts\build.ps1 -Deploy exit0. C++ 컴파일·링크와41개 Lua luac -p 구문 컴파일 통과. git diff --check 통과. Release/dist 런처·실제앱·PDB·qt.conf·두 SimConnect DLL 해시 일치,41개 Lua는 소스/Release/dist 모두 일치. 최상위 파일은 Release=EXE/README/LICENSE, dist=EXE/README/LICENSE/기존pref뿐이다. 최종 문서도 양쪽 배포 폴더에 반영했다. 실행 검증은 하지 않았다.

## 2026-11-12 — File 메뉴 순서

Close Project를 Open Recent 바로 아래로 이동하고 그 뒤 구분선으로 Save/Save All/Close File과 나눴다. 기존 닫기 동작과 미저장 확인은 유지한다. build.ps1 -Deploy exit0, 컴파일·링크 및 diff --check 통과. Release/dist 갱신 완료. 앱 실행·테스트·커밋 없음.

## 2026-11-12 — 0.2.0 로컬 체크포인트

사용자 요청에 따라 현재까지의 누적 구현을0.2.0으로 올리고 한 번의 로컬 커밋으로 보존한다. CMake PROJECT_VERSION을 기준으로 Qt applicationVersion/About을 연결하고, 실제 앱과 루트 런처 모두 Windows VERSIONINFO FileVersion/ProductVersion0.2.0 리소스를 생성한다. 프로젝트/pref 파일 스키마 version1은 변경하지 않는다. README 버전도 갱신했다. 소스·기본 Lua·문서·빌드 스크립트만 선택적으로 stage하며 .deps/build/dist/pref/로그/DLL은 Git에 추가하지 않는다.

다음 순서: DCS 연결 구현 → 충분한 실기 테스트 → 사용자 지시에 따른 GitHub 연결·푸시·배포. 이번에는 DCS 구현/설치나 GitHub 연결/푸시/릴리스/태그 생성은 하지 않는다. SimConnect 공개 재배포 권리 확인은 기존 미해결 과제로 유지한다.

DCS 검토: Saved Games의 활성 DCS 프로필 Scripts/Export.lua에서 Stella 전용 exporter를 로드하고 LuaExportStart/AfterNextFrame/Stop 등의 콜백으로 샘플을 수집해 localhost UDP로 전달하는 방안을 설명한다. 기존 Export.lua와 콜백 체인을 보존해야 한다. DCS-BIOS의 설치 안내 및 BIOS.lua 실제 구현이 로더 추가, LuaSocket, 기존 콜백 체인, export_rate 기반 실행 사례다(https://github.com/DCS-Skunkworks/dcs-bios 및 Scripts/DCS-BIOS/BIOS.lua). DCS 내 exporter는 텔레메트리 수집/전송만, Stella의 Lua는 효과 판정, C++은 수신/유효기간/실행/파형/HID를 맡는 제안이다. 기본20Hz와 필요한 빠른 항목50Hz는 검증 전 제안값이며 게임 프레임/시뮬레이션 시간의 제약을 받는다. 기체 이름/세션/순번/모델 시간과 수신 시각으로 재연결·pause·오래된 입력을 처리해야 한다. 기체별 접지/서스펜션·시동·경고 신호는 제공 여부·단위를 확인하고 미지원은 nil로 유지한다. 다른 게임과 공통 텔레메트리로 추상화하지 않는다. 멀티플레이에서는 서버의 export 허용 설정도 영향이 있으므로 자체항공기 데이터 사용 범위를 확인해야 한다(Tacview 공급자 문서 https://raia-software-inc.gitbook.io/tacview/flight-simulator/dcs-world/multiplayer-flights-as-host).

0.2.0 검증: build.ps1 -Deploy exit0, C++ 컴파일·링크 및41개 Lua 구문 컴파일 통과. Release/dist 런처와 실제 EXE 네 파일의 FileVersion/ProductVersion이 모두0.2.0, 런처/앱/PDB 해시 일치. 앱 실행·게임·HID·런타임 테스트는 수행하지 않았다. 이번 상태를 사용자 요청의 로컬 커밋으로 기록한다.

## 2026-11-12 — 승인된 별 아이콘 적용

사용자가 선택한 두 번째 이미지(노란 통통한 네 갈래 별, 하늘색 진동선, 기존 남색 둥근 배경)를 assets/stella-vibra.png로 원본 그대로 복사했다. scripts/make-icon.ps1은 System.Drawing으로 투명도를 유지하며16/20/24/32/48/64/128/256px PNG 프레임을 포함한 ICO를 생성한다. 이미지를 새로 그리거나 색/형태를 수정하지 않았다. src/version.rc.in의 ICON 리소스는 런처와 실제 EXE에 공통 적용하며 Qt 리소스의 PNG를 QApplication::setWindowIcon으로 앱 창·대화상자에 사용한다. 외부 파일 없이 실행 가능하다. 버전0.2.0 유지. 프로젝트 로고는 동일 심볼을 권장하고 이름을 옆에 붙이는 가로형은 제안만 했으며 추가 도안 생성이나 README 로고 배치는 하지 않았다. 이번 작업의 커밋·푸시는 요청되지 않아 하지 않는다. 앱/게임/HID 실행 없이 컴파일·리소스 구조·배포 해시·diff를 점검한다.

검증 결과: build.ps1 -Deploy exit0. C++/RC/Qt 리소스 컴파일·링크 통과. 원본 PNG1254x1254 및 투명 모서리 확인, ICO8개 프레임 구조 확인. EXE를 실행하지 않고 AS_DATAFILE로 읽어 dist 런처와 실제 앱의 아이콘 그룹 리소스ID1 존재 확인. 두 EXE의 Release/dist 해시 일치, diff --check 통과. 실제 탐색기/작업표시줄 렌더링은 사용자 확인으로 남긴다.

## 2026-11-12 — 프로젝트 로고와 README

승인 아이콘을 참조해 내장 image_gen으로 가로형 로고 assets/stella-vibra-logo.png를 생성했다. Stella는 노란색, Vibra는 하늘색이며 배경은 남색이다. 밝은 README 테마에서 노란 글씨의 대비가 낮아지는 것을 막기 위해 배경을 포함했다. 출력에서 철자/색/심볼 구성을 육안 확인했다. README 제목 아래720px 폭과 대체 텍스트로 등재했고 CMake install에 로고를 추가하여 Release/dist README의 상대경로도 작동하게 한다. assets/README.md에 용도·생성 방식·원문 프롬프트를 기록했다. 기존 앱 아이콘·버전은 유지, 커밋/푸시/앱 실행은 하지 않는다.

검증: build.ps1 -Deploy exit0, diff --check 통과. 소스/Release/dist 로고 및 README 해시 일치, 상대 이미지 경로 존재 확인. 생성된 로고의 Stella Vibra 철자와 배색을 확인했다. GitHub 업로드나 브라우저 README 렌더링 검증은 하지 않았다.

## 2026-11-12 — 0.2.0 사용자 문서와 릴리스 정리

기존 한국어 README를 docs/DEVELOPMENT.md로 이동하고 루트 README.md를 GitHub 사용자용 영어 소개·시작 안내로 교체했다. docs/MANUAL.md에는 프로젝트/저장, 상속·라우팅, 게임 연결, Lua API, 설정·진단을 설명했다. docs/RELEASE-NOTES-0.2.0.md와 내부 RELEASE-CHECKLIST.md는 현재 네 게임의 기능 범위, 검증 한계, 수동 확인 및 공개 배포 전 조건을 구분한다. THIRD-PARTY-NOTICES.md도 docs로 옮기고 영어로 정리했다. AGENTS/WORKFLOWS의 문서 역할과 읽기 경로를 갱신했다. 이전 DCS 선행 계획 대신 최신 요청에 따라 현재 기능을0.2.0으로 고정하며 DCS는 후속 작업이다.

Help → README / User Manual(F1)에 비모달 오프라인 Markdown 뷰어를 추가했다. Back/Forward/Home과 문서 상대 링크를 지원하며 웹 링크는 사용자가 클릭하면 브라우저에서 연다. CMake install로 사용자 문서·로고를 Release/dist에 동봉한다. 이전 배포 폴더의 내부 CONTRACTS/SIMCONNECT-LICENSING 문서만 정리하고 사용자 pref·프로젝트·진단은 보존한다. 임시 --ui-check 자동 레이아웃 확인 분기를 제거했으며 일반 로그/크래시 덤프는 유지한다. 승인된 아이콘·프로젝트 로고의 앞선 미커밋 변경도 이번 사용자 요청에 따른 로컬 커밋에 포함한다. GitHub 연결/푸시/태그/공개 업로드는 수행하지 않는다.

검증: .\scripts\build.ps1 -Deploy exit0. C++/RC 컴파일·링크 및41개 Lua 구문 컴파일 통과. 사용자 문서/이미지 상대경로18개 존재 확인, 소스와 Release/dist 문서·로고 해시 일치, 두 배포 폴더의 런처/앱/PDB 해시 일치, EXE 네 파일 ProductVersion0.2.0 확인. diff --check 통과. 앱 실행·테스트·게임·HID/모터 동작은 수행하지 않았다. Help 렌더링·페이지 이동·F1과 최종 패키지 실기 회귀 검증은 사용자 확인 항목이다. 실행 경로: build/Release/StellaVibra.exe 또는 dist/StellaVibra/StellaVibra.exe.

공개 배포의 미해결 항목: 공식 SimConnect 클라이언트의 공개 재배포 권리는 아직 확인되지 않았다. 소스 MIT와 SDK DLL 권리는 별개이며, 현재 dist는 로컬 실행용 준비 폴더다. 공개 패키징 시 개인 pref/diagnostics/프로젝트를 제외해야 한다. 해당 내용을 영어 릴리스 노트·제3자 고지와 내부 체크리스트에 명시했다.

## 2026-11-12 — SimConnect 동봉 배포 방침 및 GitHub 준비

사용자 결정에 따라 명시적 허가 회신을 기다리는 운영 조건을 해제하고 원본 SimConnect 두 버전을 포함하는 Windows 패키지를 준비했다. README/매뉴얼/릴리스 노트/제3자 고지/About, SIMCONNECT-LICENSING 및 운영 문서를 일치시켰다. Microsoft Corporation 및 라이선서의 권리, Stella MIT 적용 제외, 비보증/비지원 관계를 표시하고 명시적 재배포 허가 미확인은 제3자 고지에 보존한다. 이의 제기 시 다운로드 철회/패키징 재검토 방침을 기록했다. 소유권 고지·철회가 허가나 소급 면책을 의미한다고 표현하지 않는다. 공식 SDK EULA와 GitHub Releases 공식 문서를 다시 확인했다.

scripts/package.ps1은 build.ps1 -Deploy 이후 실행한다. 배포 대상 파일 목록만 새 dist/packages/<GUID>/StellaVibra에 복사하여 Windows ZIP 및 SHA-256 파일을 만든다. 원본 DLL 해시와 현재 문서/템플릿/라이선스의 배포 일치를 검사하고 두 SimConnect 클라이언트·EULA·provenance, Qt/Lua 대응 소스, 실행 파일/PDB, 도움말을 포함한다. 사용자 실행 폴더는 보존하며 pref/diagnostics/프로젝트는 압축하지 않는다. docs/PUBLISHING.md에 Git 소스와 Release asset 차이, Draft/Pre-release, 태그/Target, ZIP 업로드 및 철회 절차를 한국어로 작성했다.

검증: build.ps1 -Deploy exit0, 컴파일·링크/Lua 구문 검사 통과. package.ps1 exit0. 산출물 dist/packages/234cd71cc6c140428ab509921b99eab7/StellaVibra-0.2.0-windows-x64.zip (67,438,361 bytes), SHA-256 08f67f1d57d3f2c4873f229e442488c05c408e38641accb804b28f6cfba54ff3. ZIP 내124개 파일의 해시와 staging 일치, 필수 DLL/EULA 포함, pref/log/dmp/diagnostics/sv/Git 파일 미포함 확인. diff --check 통과. 앱/게임/HID 실행 및 런타임 테스트 미수행. 커밋·태그·푸시·GitHub 업로드는 이번에 요청되지 않아 수행하지 않았다. 공개 전 이번 변경을 커밋하고 그 커밋과 패키지를 대응시켜야 한다.

## 2026-11-13 — GitHub 최초 소스 푸시

사용자가 https://github.com/PillarofHub/StellaVibra.git 로 현재 작업 커밋 및 푸시를 명시적으로 요청했다. 이전 푸시 금지 기본 규칙의 이번 작업 예외다. 미커밋 SimConnect 동봉 고지·패키징 스크립트·공개 절차를 선택 커밋한다. 원격 main의 초기 MIT LICENSE 커밋은 강제 덮어쓰기 없이 병합하고 PillarofCode 및 Stella Vibra contributors 저작권 표기를 함께 보존한다. 태그/Target/Release 설정과 바이너리 업로드는 후속 작업이다. 이전 빌드·패키지 검증 결과를 유지하며 이번에는 diff와 Git 이력을 확인한다. 라이선스 고지가 변경되므로 공개 바이너리는 다음 릴리스 단계에서 build.ps1 -Deploy 및 package.ps1로 재생성해야 한다.

푸시 결과: 로컬 변경 커밋2cec489 및 원격 초기 이력 병합8a2009a 완료. main 푸시는 GitHub403으로 거절되었다: 현재 HTTPS 인증 계정 honey-yi-papaya에 PillarofHub/StellaVibra 쓰기 권한이 없다. 원격 main은 ed3f110 그대로이며 태그/릴리스도 생성하지 않았다. 저장소 쓰기 권한 계정으로 Git 인증을 변경하거나 해당 계정에 권한을 부여한 뒤 main 푸시를 재시도해야 한다. 비밀번호/토큰을 대화에 보내지 않는다.

## 2026-11-13 — Windows PowerShell 5.1 빌드/패키징 호환 수정

사용자 오류를 확인: BOM 없는 UTF-8 스크립트의 한글 정규식이 Windows PowerShell에서 손상되어 catch 자체가 실패했다. 오류 필터를 ASCII Unicode escape로 변경했다. 가려진 원인은 windeployqt의 번역 경고(stderr)를 ErrorActionPreference Stop이 종료 오류로 취급한 것이다. 해당 호출에서만 Continue로 경고를 기록하고 종료 코드로 실패를 판정하도록 변경했다. build/package의 .NET Framework 미지원 Path.GetRelativePath를 이미 열거된 하위 파일의 접두 경로 제거로 대체했다.

검증: Windows PowerShell 5.1 powershell.exe -NoProfile -ExecutionPolicy Bypass -File scripts/build.ps1 -Deploy exit0, package.ps1 exit0. 새 패키지 dist/packages/a160a487faa140a883624d8b95664c09/StellaVibra-0.2.0-windows-x64.zip, 124파일. SHA256 6402d5568dbe5a4c75918bd96252719f022459d919f9c7ddb3237010e643b585. diff --check 통과. 앱/게임/HID/테스트 실행 없음. 이번 수정 커밋·푸시 미수행; 릴리스 태그 생성 전 수정 커밋을 포함해야 한다.

## 2026-11-13 — 호환 수정 푸시 및 최종 패키지 생성

사용자 요청으로 e7aa389(PowerShell 5.1 호환 수정)을 origin/main에 푸시 완료. Windows PowerShell에서 build.ps1 -Deploy 및 package.ps1 모두 exit0. 업로드할 최신 ZIP/SHA-256은 dist/packages/eb9ada04180e4cbda6ce74bf8a9e1ecd/에 있다. ZIP SHA256 ed05bb0b7ea7be8e6a00a29dfe50e863d46d0abd8cea2b45c0a207d8e7023aeb, 124파일. 생성된 체크섬 파일과 ZIP 해시 일치 확인. 실행 테스트·태그·Release 생성·바이너리 업로드는 하지 않았다. 이 기록 후속 커밋은 문서만 변경하며 패키지의 코드 기준은 e7aa389과 동일하다.
