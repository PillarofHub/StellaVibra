# GitHub 공개 절차 (유지관리자용)

소스 저장소는 코드와 문서를, Releases는 특정 Git 태그에 대응하는 실행 패키지와 변경 설명을 제공한다. GitHub의 자동 Source code ZIP은 실행 파일이 아니므로 별도 Windows ZIP을 asset으로 올려야 한다.

## 로컬 준비

1. 변경 파일을 검토하고 명시적으로 선택하여 커밋한다. DLL/.deps/build/dist/pref/diagnostics는 소스 Git에 넣지 않는다.
2. `.\scripts\build.ps1 -Deploy` 후 `.\scripts\package.ps1`을 실행한다. 새 dist/packages/<고유ID>/ 아래 Windows ZIP과 .sha256을 만든다. 기존 실행 폴더는 변경하지 않는다. 원본 SimConnect 두 버전, EULA, 출처, Qt/Lua 대응 소스, 고지, 매뉴얼을 포함하고 pref/진단/사용자 프로젝트는 제외한다.
3. 출력 ZIP과 대응 소스 커밋을 함께 보관한다. ZIP을 새 폴더에 풀어 수동 검증한다. 기존 MSFS2024 성공 사례와 다른 게임의 미검증 상태를 구분한다. 배포 ZIP에는 덤프 분석용 PDB가 포함된다.
4. 사용자 결정에 따라 SimConnect DLL을 동봉한다. 명시적 재배포 허가를 확보했다는 의미는 아니며 docs/THIRD-PARTY-NOTICES.md의 고지와 EULA를 보존한다.

## 저장소와 Releases

1. GitHub에서 빈 저장소를 만들거나 사용할 저장소를 선택한다. 기존 로컬 이력을 올릴 새 저장소라면 README/License 자동 생성을 생략한다.
2. `git remote -v`로 원격을 확인한다. 원격이 없다면 `git remote add origin <저장소-URL>`을 실행하고, 공개할 파일/이력을 검토한 뒤 `git push -u origin main`으로 올린다. 이미 origin이 있으면 무조건 덮어쓰지 않는다.
3. 저장소의 Releases → Draft a new release. 태그는 `v0.2.0`, Target은 패키지를 만든 커밋이 있는 브랜치다. 브랜치가 그 후 변경되었다면 정확한 커밋에 태그를 만들어 push한 뒤 기존 태그를 선택한다.
4. 제목은 `Stella Vibra 0.2.0`. 영어 릴리스 노트의 기능·검증 범위·제약을 본문에 옮긴다. 파일 내 상대 Markdown 링크는 GitHub 저장소의 해당 태그 파일 URL로 바꾼다.
5. `StellaVibra-0.2.0-windows-x64.zip`과 `.zip.sha256`을 첨부한다. 디렉토리나 EXE 단독을 올리지 않는다.
6. 최종 실기 검증 전에는 **This is a pre-release**를 권장한다. 먼저 **Save draft**로 저장해 내용과 첨부 파일을 확인한다. **Publish release**를 누르면 공개된다. Draft는 미게시 상태, Pre-release는 공개된 시험판 표시다.

문제가 생기면 해당 다운로드를 제거하거나 릴리스를 삭제하고 사유를 알린다. 원격 다운로드 삭제로 이미 받은 복사본이 회수되지는 않는다. 저장소에서 immutable releases를 켠 경우 게시 후 asset 교체·삭제가 제한되므로 이 정책을 먼저 확인한다. 일반 버그 수정은 같은 태그의 ZIP을 조용히 바꾸기보다 새 버전/태그로 배포한다.

공식 안내: https://docs.github.com/en/repositories/releasing-projects-on-github/managing-releases-in-a-repository (2026-11-12 확인).

이 문서는 절차 안내다. 자동 커밋·태그·푸시·Release 생성은 수행하지 않는다.
