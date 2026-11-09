# SimConnect 확보·배포 검토

검토일: 2026-11-09. 이 문서는 확인한 자료와 프로젝트의 배포 판단을 기록한다. Microsoft의 법적 허가를 대신하지 않는다.

## 확인 결과

- [공식 SDK 다운로드 목록](https://sdk.flightsimulator.com/msfs2024/files/sdk.json)은 retail 경로에 SDK Core 1.7.3, flighting 경로에 1.8.2를 제공한다. 현재 개발용 원본은 retail 1.7.3에서 확보한다. [공식 SDK 안내](https://docs.flightsimulator.com/msfs2024/retail/introduction/sdk-overview/)는 게임 Developer Mode의 Help 메뉴에서도 SDK를 제공한다고 설명한다.
- [SDK EULA](https://docs.flightsimulator.com/msfs2024/retail/introduction/sdk-eula/) 1(a)는 애드온 개발 목적 사용을 허용하고 2(e)는 배포 가능한 코드의 예외를 둔다. 그러나 그 페이지는 네이티브 SimConnect.dll을 그 예외에 해당하는 파일로 명시하지 않는다. 다운로드 가능하다는 사실만으로 외부 재배포 허가가 확정되지는 않는다.
- [Asobo의 2026-10-28 답변](https://devsupport.flightsimulator.com/t/is-the-unmodified-simconnect-dll-distributable-code-for-a-commercial-external-add-on/18701/17)은 이 질의를 Microsoft에 전달했으나 답변이 없다고 설명한다. 2026-11-08 마지막 게시물(#24)까지 확인했으며 이후 게시물에도 명시적 허가 답변은 없었다. 커뮤니티의 실제 배포 사례나 빌드 시 DLL 복사 설정은 참고가 되지만 권한 있는 배포 허가의 대체 근거로 사용하지 않는다.
- [Asobo의 기술 답변](https://devsupport.flightsimulator.com/t/simconnect-dll-is-now-simconnect-internal-dll-in-msfs-2024/13559/2)에 따르면 게임의 SimConnect_internal.dll은 SDK 클라이언트 DLL과 같은 프로세스 내 클라이언트 버전 처리 방식이 다르다. 게임 설치 경로를 찾는 것만으로 정식 SDK 클라이언트 확보 절차를 대체한다고 보장할 수 없다. 파일 이름 변경·WindowsApps 권한 변경·게임 내부 DLL 복사는 구현하지 않는다.

## 이번 구현의 선택

로컬 개발용 Release에는 공식 SDK에서 추출한 원본 DLL을 둘 수 있도록 한다. 이 로컬 개발 복사는 공개 재배포 권리 확인과 구분한다. Aurora의 DLL은 원 SDK 버전이 없고 SHA-256이 `cae445bddaf35c6f75f01ed415cf708c61fc3a0cbc2412f577f768d82f143b30`인 별도 바이너리이므로 공식 2024 파일이라고 표시하지 않는다.

앱은 Connect MSFS를 누를 때 사용자 설정 절대 경로 → 설정이 비었으면 EXE 옆 SimConnect.dll → MSFS2024_SDK 환경변수의 SimConnect SDK/lib → C:/MSFS 2024 SDK/SimConnect SDK/lib 순서로 찾는다. 명시한 설정 경로가 잘못되면 다른 DLL로 조용히 대체하지 않고 오류를 표시한다. 현재 작업 폴더·무제한 PATH·게임 내부 디렉토리는 검색하지 않는다. 자동 탐색은 자동 연결이나 모터 시작을 의미하지 않는다.

`dist/StellaVibra`는 Microsoft DLL을 자동 포함하지 않는다. 프로젝트 MIT 라이선스는 Microsoft DLL에 적용되지 않는다. MIT 고지나 Microsoft 저작권 문구를 덧붙이는 것만으로 재배포 권한이 새로 생기지 않는다.

## 공개 배포 전 해결 방법

1. 정확한 SDK 버전·DLL 크기·SHA-256·원본 URL을 유지한다. 원본 DLL과 고지를 수정하지 않는다.
2. Microsoft/Asobo에 이 파일이 SDK EULA의 distributable code인지, 외부 무료/유료 앱에 동봉 가능한지, 필수 고지와 업데이트 의무가 무엇인지 서면 확인한다. MSI 또는 EXE에 내장하는 방식도 별도로 확인한다. 이 작업에서는 문의를 전송하지 않았다.
3. 허가가 확인되면 그 조건에 맞춰 dist 포함을 활성화한다. 미확인 상태에서는 Microsoft DLL 없이 배포하고 정식 SDK/승인된 런타임 위치를 선택하는 방식으로 안내한다. 일반 최종 사용자가 개발 목적 없이 SDK를 사용하는 조건까지 확정했다고 표현하지 않는다.
4. 공식 다운로드를 앱이 대신 실행하거나 사용자가 게임 내부 DLL을 직접 선택하게 하는 방식도 라이선스 문제를 자동으로 해결하지 않는다. 정식 클라이언트 위치와 사용 조건을 명확히 하는 것이 우선이다.

## 확보한 원본

- 공식 파일: https://sdk.flightsimulator.com/msfs2024/files/installers/1.7.3/MSFS2024_SDK_Core_Installer_1.7.3.zip
- ZIP SHA-256: `aae63270c9420e099ffa2d21565c267f0603f8e226cf2c8ab2ca8966f6a052bf`.
- DLL: 84,480 bytes, x64, SHA-256 `b10de7adf4c62e5f66c89dd6d01b64091bbbcec83411cfe191d6b85fbee61d15`.
- Authenticode: NotSigned. 공식 HTTPS 서버에서 받은 아카이브와 추출 경로로 출처를 기록했으며 Microsoft 전자서명이 있다고 주장하지 않는다.
- MSI를 실행/설치하지 않고 File/Media 테이블을 읽어 cab5의 네이티브 DLL과 헤더, cab1의 EULA PDF를 추출했다. `.deps/msfs2024/client`에 보관한다. SDK 전체 설치/레지스트리 변경은 하지 않았다.
- `build/Release/SimConnect.dll`, `SimConnect-provenance.json`, `licenses/MSFS SDK EULA.pdf`가 로컬 개발용 파일이다. DLL·SDK 헤더·EULA PDF 자체는 Git과 공개 dist에 포함하지 않는다.
- 사용된 헤더가 있으면 CMake가 ABI static_assert를 활성화한다. 이는 컴파일 검사이며 DLL을 로드하거나 게임에 연결하지 않는다.