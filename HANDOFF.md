# Stella Vibra 인수인계

## 현재 상태 — 2026-11-09

기준 HEAD `9b399f3`에서 첫 MVP W1–W6을 구현했다. 시작 시 작업 트리는 깨끗했고, 사용량 한도 후 재개 시 이번 작업의 미커밋 변경을 보존했다. 이후 사용자가 MVP와 SimConnect 확보·라이선스 검토 변경의 커밋을 명시적으로 요청했다. 이번 작업을 커밋하며 푸시는 하지 않는다. Aurora 파일은 변경하지 않았고 하위 에이전트·별도 채팅도 만들지 않았다.

### 구현

- C++20/MSVC, Qt Widgets 6.8.3, CMake, Lua 5.4.8. 소스 MIT 및 타사 고지 포함.
- Project Explorer와 실제 폴더 트리, Parameters/Script Editor 상하 분할, Motor Monitor, Output, 연결/실행 상태. Lua 행 번호·문법 강조·검색·오류 위치 이동.
- 프로젝트 생성·열기·원자적 파일별 저장, 효과 등록, 글로벌→기체 Override, 필드별 Reset to Global, PC 공통 논리 슬롯 매핑. 미해결 설정 키 보존.
- `init(api)`, `update(context)`, self 상태, 다중 이름 기반 포트, numeric/boolean 파라미터, 제한된 라이브러리·8MiB 할당·명령 수/시간 검사. Lua도 C++로 컴파일하여 보호 호출 오류 시 호스트 객체를 해제한다.
- Save/Apply 분리, 전체 프로필 검증 후 실행 버전 교체, 실패 시 기존 버전 유지. 프로젝트 전환은 이전 실행 버전을 해제한다. Disable/Stop 즉시 출력 정리.
- UI/실행기/HID 스레드 분리. 20ms PWM 양자화·위상·우선순위, 포트별 기한 교체, 500ms 입력 만료와 250ms heartbeat. 명시적 Start와 최대 5초 모터 테스트.
- 실제 `effects/ground_roll.lua`, `effects/touchdown.lua`. 접촉·첫 압축 상승·바퀴별 재무장·기간 계산은 Lua에 있다.
- MSFS 2024 공개 SimConnect ABI 동적 로딩, TITLE 기반 기체 선택, 원 수신 시각 보존, 중복 패킷 제거, pause/시뮬레이션 시각 정체 처리.
- HID MVP 지원은 Combat Throttle Metal.EX (`4098:B970`) 한 모델이다. Aurora HANDOFF의 A=Left/B=Right와 프로토콜을 근거로 구현했다. descriptor에서 report 02/길이를 확인한 인터페이스에만 출력한다. 일련번호 부재/중복은 자동 배정하지 않는다.

## 빌드·배포

```powershell
.\scripts\bootstrap.ps1
.\scripts\build.ps1 -Deploy
```

- MSVC 19.51.36260, Windows SDK 10.0.26100.0, VS Build Tools 2026 내 CMake.
- `build\Release\StellaVibra.exe`: Qt DLL/Windows 플러그인, app-local MSVC CRT, effects, 라이선스와 대응 소스를 같은 Release 폴더에 배치한다. build.ps1 기본 실행도 이 구성을 만든다.
- `dist\StellaVibra\StellaVibra.exe`: `-Deploy`로 동일한 실행용 배포 폴더를 만든다.
- Qt/Lua 의존성은 `.deps`에만 설치했고 Git에서 제외했다. 공식 다운로드 URL과 SHA-256은 bootstrap.ps1에 고정했다.
- 컴파일·링크 및 배포 스크립트 성공(exit 0). `dumpbin /dependents`로 로컬 Release EXE/DLL 25개의 import를 점검하여 app-local/Windows System32에 없는 필수 의존성이 없음을 확인했다. `git diff --check` 통과. 신규 소스·문서도 별도 공백 점검했다. 원본 Qt 라이선스 3개 파일의 기존 후행 공백은 고지 원문 보존을 위해 수정하지 않았다.
- CMake Vulkan 헤더 미발견, windeployqt 번역 catalogs.json 및 dxcompiler/dxil 미발견 안내가 있다. 영어 Qt Widgets/raster UI에 별도 Vulkan/DirectX 렌더러를 사용하지 않으며, 정적 import에서 이 파일들은 필수 의존성이 아니다. 실제 구동 확인은 별도다.

## 남은 외부 조건·검증 한계

- 공식 서버의 retail SDK Core 1.7.3을 내려받아 네이티브 DLL·헤더·EULA만 추출했다. **로컬 build/Release에는 DLL을 복사해 Connect MSFS에서 자동 탐색한다. 공개 dist에는 DLL을 포함하지 않는다.** 자동 탐색 순서는 설정 경로(있으면 그것만) → 미설정 시 EXE 옆 → MSFS2024_SDK → 기본 SDK 폴더다. Aurora 복사본은 사용하지 않았다.
- 공개 ABI를 공식 문서와 대조했다. EXCEPTION=1, STRING256=9이며 Aurora의 해당 상수 5/12는 따르지 않았다. 공식 1.7.3 헤더로 enum과 패킷 오프셋 static_assert를 컴파일했다. 실제 SimConnect 수신은 수행하지 않았다.
- 앱 실행, GUI 확인, Lua 효과 동작 테스트, MSFS 연결, HID 프로브·테스트·모터 구동은 전혀 하지 않았다. 컴파일 성공은 동작 검증 완료를 뜻하지 않는다.
- 기체별 GEAR ANIMATION POSITION 의미/정적 반환, 접촉 신호, 초기 pause 상태, TITLE 매칭, HID descriptor와 좌우 및 전송 타이밍은 사용자 검증 필요.
- Lua 시간 제한은 1000명령마다 검사하며 네이티브 함수까지 하드 실시간 선점하지 않는다. HID heartbeat는 별도로 출력을 정리한다. 강제 프로세스 종료·드라이버 장애 시 물리 모터의 자체 정지는 확인하지 못했다.
- 자동 장치 재연결은 하지 않는다. 연결이 바뀌면 Scan Devices를 누르고 필요 시 수동 재배정한다. 스캔은 Stop을 수행한다. 동일 Lua의 다중 인스턴스, 다른 게임/장치, 리플레이 등 MVP 제외 기능은 추가하지 않았다.

## 사용자 확인 순서

1. Release EXE 실행, Stopped 확인 → 빈 폴더에 New → 효과 선택·편집·Save·재열기.
2. 파라미터/출력 매핑·기체 Override/Reset, 정상/실패 Apply와 로그·오류 위치 확인.
3. 로컬 Release의 DLL 자동 탐색(또는 Settings에서 다른 공식 DLL 선택) → MSFS 2024 비행 진입 → Connect MSFS → TITLE/입력 상태 확인.
4. Scan Devices → 실제 좌우/논리 슬롯 확인 → 낮은 세기·짧은 Test Start → 타임아웃/Stop 확인.
5. Apply 성공 후 Run → Ground Roll/Touchdown → 일시정지·기체 변경·연결 해제·Stop 시 출력 해제 확인.

후속 수정은 이 관측 결과로 범위를 정한다. 새 세션 일지 대신 이 문서를 갱신한다.

## SimConnect 라이선스 검토와 커밋 범위

- 검토일 2026-11-09. SDK EULA와 Asobo 공식 답변, 통합 스레드의 11월 8일 게시물까지 확인했다. 네이티브 DLL의 공개 재배포 허가는 아직 확인되지 않았다. MIT 고지만으로 해결되는 문제가 아니다.
- SDK DLL의 원본 해시는 `b10de7adf4c62e5f66c89dd6d01b64091bbbcec83411cfe191d6b85fbee61d15`, 크기는 84,480 bytes. 상세 출처·확보 과정·공개 배포 해결 경로는 SIMCONNECT-LICENSING.md에 기록했다.
- 공식 SDK 라이선스 아래 이 프로젝트의 로컬 애드온 개발용 Release에 원본 DLL/EULA를 복사했다. 공개 dist, Git에는 Microsoft 바이너리·SDK 헤더를 넣지 않는다. Microsoft 서면 확인 전 공개 번들링 허가를 주장하지 않는다. 외부 문의는 전송하지 않았다.
- 게임의 SimConnect_internal.dll 자동 검색/이름 변경은 사용하지 않는다. 공식 SDK 클라이언트 경로를 사용한다.
- `scripts/build.ps1 -Deploy` 컴파일·링크·배포를 재수행해 성공했다. 공식 SDK 헤더 내부의 상수 초기화에서 C4245 signed/unsigned 경고가 발생하며 SDK 원본은 수정하지 않았다. 원본/Release DLL SHA-256 일치와 dist의 DLL 제외를 확인했다. 앱·MSFS·HID 실행, 테스트 실행은 하지 않았다.
- 커밋 대상은 이전 MVP 구현 파일들과 이번 클라이언트 탐색·로컬 배치·문서 변경이다. 빌드 산출물과 내려받은 SDK는 제외한다. 정확한 커밋 ID는 git log -1로 확인한다.