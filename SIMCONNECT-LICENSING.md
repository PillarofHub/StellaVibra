# SimConnect 확보·배포 검토

검토일: 2026-11-09. 이 문서는 확인한 자료와 프로젝트의 배포 판단을 기록한다. Microsoft의 법적 허가를 대신하지 않는다.

## 확인 결과

- [공식 SDK 다운로드 목록](https://sdk.flightsimulator.com/msfs2024/files/sdk.json)은 retail 경로에 SDK Core 1.7.3, flighting 경로에 1.8.2를 제공한다. 현재 개발용 원본은 retail 1.7.3에서 확보한다. [공식 SDK 안내](https://docs.flightsimulator.com/msfs2024/retail/introduction/sdk-overview/)는 게임 Developer Mode의 Help 메뉴에서도 SDK를 제공한다고 설명한다.
- [SDK EULA](https://docs.flightsimulator.com/msfs2024/retail/introduction/sdk-eula/) 1(a)는 애드온 개발 목적 사용을 허용하고 2(e)는 배포 가능한 코드의 예외를 둔다. 그러나 그 페이지는 네이티브 SimConnect.dll을 그 예외에 해당하는 파일로 명시하지 않는다. 다운로드 가능하다는 사실만으로 외부 재배포 허가가 확정되지는 않는다.
- [Asobo의 2026-10-28 답변](https://devsupport.flightsimulator.com/t/is-the-unmodified-simconnect-dll-distributable-code-for-a-commercial-external-add-on/18701/17)은 이 질의를 Microsoft에 전달했으나 답변이 없다고 설명한다. 2026-11-08 마지막 게시물(#24)까지 확인했으며 이후 게시물에도 명시적 허가 답변은 없었다. 커뮤니티의 실제 배포 사례나 빌드 시 DLL 복사 설정은 참고가 되지만 권한 있는 배포 허가의 대체 근거로 사용하지 않는다.
- [Asobo의 기술 답변](https://devsupport.flightsimulator.com/t/simconnect-dll-is-now-simconnect-internal-dll-in-msfs-2024/13559/2)에 따르면 게임의 SimConnect_internal.dll은 SDK 클라이언트 DLL과 같은 프로세스 내 클라이언트 버전 처리 방식이 다르다. 게임 설치 경로를 찾는 것만으로 정식 SDK 클라이언트 확보 절차를 대체한다고 보장할 수 없다. 파일 이름 변경·WindowsApps 권한 변경·게임 내부 DLL 복사는 구현하지 않는다.

## 초기 구현의 선택 (이력)

로컬 개발용 Release에는 공식 SDK에서 추출한 원본 DLL을 둘 수 있도록 한다. 이 로컬 개발 복사는 공개 재배포 권리 확인과 구분한다. Aurora의 DLL은 원 SDK 버전이 없고 SHA-256이 `cae445bddaf35c6f75f01ed415cf708c61fc3a0cbc2412f577f768d82f143b30`인 별도 바이너리이므로 공식 2024 파일이라고 표시하지 않는다.

후속 UI 요청에 따라 Settings 경로 선택을 제거했다. 앱은 배포 루트의 bin/simconnect/MSFS2024/SimConnect.dll 또는 bin/simconnect/MSFS2020/SimConnect.dll만 로드하며 MSFS 연결을 자동 재시도한다. 게임 내부 디렉토리·환경변수·PATH 검색은 사용하지 않는다.

현재 build/Release와 dist/StellaVibra는 모두 로컬 실행 검증용 준비 폴더로, 공식 SDK DLL·출처·EULA를 포함한다. 공개 업로드나 배포는 수행하지 않았다. 이 파일 배치 변경은 아래의 공개 재배포 권리 미확인 결론을 변경하지 않는다. 프로젝트 MIT 고지는 Microsoft DLL에 적용되지 않는다.

## 현재 배포 방침 (2026-11-12 사용자 결정)

사용자는 별도 허가 회신을 기다리지 않고 원본 SimConnect SDK 클라이언트를 Windows 패키지에 포함하기로 결정했다. 이전의 “확인 전 공개 배포하지 않음” 운영 방침은 이 결정으로 대체한다. 이는 Microsoft가 허가했다는 판단이나 법률적 보장이 아니다. 공식 EULA의 distributable code 예외는 존재하지만 해당 DLL의 적용 여부에 대한 명시적 확인은 여전히 확보하지 못했다. 소유권 표기와 다운로드 철회만으로 위험이 없어지거나 이전 배포 책임이 해소된다고 보지 않는다.

- 2024/2020 원본 DLL·SHA-256·SDK 버전·출처 JSON·EULA를 유지한다. 게임 내부 DLL이나 Aurora의 바이너리를 대신 배포하지 않는다.
- 사용자용 제3자 고지와 About에 Microsoft Corporation 및 라이선서의 권리, Stella MIT 적용 제외, Microsoft 비보증/비지원 관계를 표시한다. 제3자 고지에 명시적 허가 미확인과 유지관리자의 동봉 결정을 구분해 기록한다.
- Microsoft의 이의 제기가 있으면 해당 Release 다운로드를 철회하고 DLL 제외/설치 방식 변경 등을 재검토한다. 이미 다운로드된 복사본은 회수되지 않으며 철회가 소급 허가가 되는 것은 아니다.
- DLL·SDK 원본은 소스 Git에 넣지 않고 바이너리 Release asset에 포함한다. 이번 작업에서 GitHub 업로드나 문의 전송은 하지 않는다.

공식 EULA 재확인: https://docs.flightsimulator.com/msfs2024/retail/introduction/sdk-eula/ (2026-11-12). 아래 확보 기록의 로컬 개발용 표기는 당시 확보 목적을 뜻한다. 현재 배포 정책은 이 절을 따른다.

## 확보한 원본

- 공식 파일: https://sdk.flightsimulator.com/msfs2024/files/installers/1.7.3/MSFS2024_SDK_Core_Installer_1.7.3.zip
- ZIP SHA-256: `aae63270c9420e099ffa2d21565c267f0603f8e226cf2c8ab2ca8966f6a052bf`.
- DLL: 84,480 bytes, x64, SHA-256 `b10de7adf4c62e5f66c89dd6d01b64091bbbcec83411cfe191d6b85fbee61d15`.
- Authenticode: NotSigned. 공식 HTTPS 서버에서 받은 아카이브와 추출 경로로 출처를 기록했으며 Microsoft 전자서명이 있다고 주장하지 않는다.
- MSI를 실행/설치하지 않고 File/Media 테이블을 읽어 cab5의 네이티브 DLL과 헤더, cab1의 EULA PDF를 추출했다. `.deps/msfs2024/client`에 보관한다. SDK 전체 설치/레지스트리 변경은 하지 않았다.
- `build/Release/bin/simconnect/MSFS2024/SimConnect.dll`, 같은 폴더의 `provenance.json`, `licenses/MSFS SDK EULA.pdf`가 로컬 개발용 파일이다. 동일 파일을 로컬 dist에도 배치한다. Microsoft 바이너리·SDK 헤더·EULA PDF는 Git에 포함하지 않는다.
- 사용된 헤더가 있으면 CMake가 ABI static_assert를 활성화한다. 이는 컴파일 검사이며 DLL을 로드하거나 게임에 연결하지 않는다.

## MSFS 2020 로컬 클라이언트 추가 (2026-11-11)

- [공식 SDK 목록](https://sdk.flightsimulator.com/files/sdk.json)의 MSFS 2020 Core 0.24.6.0을 확보했다. [원본 ZIP](https://sdk.flightsimulator.com/files/installers/0.24.6/MSFS_SDK_Core_Installer_0.24.6.0.zip)의 SHA-256은 `6DAAA1FA45598D890D17B1C8347CD736E529C83BFC00630FCEFE8822500EAF3F`다.
- MSI를 실행하지 않고 File/Media 테이블을 읽어 cab1에서 네이티브 DLL·헤더·EULA를 추출했다. DLL은 67,072 bytes, SHA-256 `CFFB74B62B596A897B0FF101BAC3D66EA10236C540C75864658B451FFD1E6513`다.
- `.deps/msfs2020/client`에 원본·provenance.json을 보관한다. Release/dist의 `bin/simconnect/MSFS2020/`에 DLL·출처 JSON·SDK EULA를 동봉하며 2024 클라이언트를 덮어쓰지 않는다. 프로젝트 게임에 맞는 라이브러리만 로드한다. 게임 내부 파일·Aurora 바이너리는 복사하지 않았다.
- 이 확보·로컬 개발용 복사는 공개 재배포 승인을 의미하지 않는다. 기존 Microsoft 권리 확인 과제는 두 클라이언트 모두에 남는다. DLL·헤더·SDK 원본은 Git에 넣지 않는다.
