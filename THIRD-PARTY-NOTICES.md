# 타사 구성요소

Stella Vibra 자체는 MIT다. 아래 구성요소는 별도 조건을 따른다.

- Qt 6.8.3: 수정하지 않은 QtCore/QtGui/QtWidgets 동적 라이브러리 및 Windows 플러그인. LGPL-3.0 사용. `licenses/LGPL-3.0.txt`, `licenses/GPL-3.0.txt`, `licenses/Qt-THIRD-PARTY.md` 및 함께 제공하는 `licenses/qtbase-LICENSES` 참조. 사용자는 DLL 교체·수정 및 이를 위한 디버깅/역공학을 할 수 있다. [정확한 소스](https://download.qt.io/archive/qt/6.8/6.8.3/submodules/qtbase-everywhere-src-6.8.3.tar.xz), [Qt 라이선스 안내](https://doc.qt.io/qt-6.8/licensing.html). 공개 재배포 시 해당 소스와 고지를 수신자에게 함께 제공해야 한다. 로컬 빌드 캐시 `.deps`는 Git에 포함하지 않는다.
- Lua 5.4.8: MIT, Copyright © 1994–2025 Lua.org, PUC-Rio. `licenses/Lua-5.4.8.html`. [공식 소스](https://www.lua.org/ftp/lua-5.4.8.tar.gz).
- HID 프로토콜·MSFS 변수 선택은 Aurora Vibra의 MIT 참고 구현에서 확인했다. Copyright (c) 2025 Rodrigo Troncoso, Copyright (c) 2026 chegevarko1982. 고지는 `licenses/Aurora-MIT.txt`. Aurora의 파형 혼합 로직은 이 앱에 사용하지 않는다.
- Microsoft Visual C++ runtime: Microsoft 라이선스 적용. 설치된 Visual Studio Build Tools의 재배포 가능 CRT DLL을 app-local로 배포한다.
- Microsoft SimConnect: Microsoft 소유 구성요소다. 로컬 실행 검증용 build/Release와 dist/StellaVibra에는 공식 SDK Core 1.7.3에서 추출한 원본 DLL을 별도 조건으로 복사한다. 앱은 EXE 옆 클라이언트만 사용한다. 현재 준비 폴더의 공개 재배포 권리는 확인되지 않았으며 공개 배포는 수행하지 않았다. 원본 출처·해시는 로컬 SimConnect-provenance.json, 재배포 검토는 SIMCONNECT-LICENSING.md를 따른다. SDK가 설치되어 있지 않아 공개 문서의 최소 C ABI를 선언해 동적으로 로드한다. Aurora에 포함된 DLL은 원 SDK 버전과 재배포 근거가 불명확해 복사하지 않았다.

공식 qtbase 바이너리 아카이브 SHA-256: `41688269fac0565db956c66d9eecae777d16197e0c02cd81b88640c1f5d73d3f`.
Lua 소스 SHA-256: `4f18ddae154e793e46eeab727c59ef1c0c0c2b744e7b94219710d76f530629ae`.
