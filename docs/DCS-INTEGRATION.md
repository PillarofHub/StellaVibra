# DCS 설치·전송 계약 (프로토콜 1)

2026-11-14. 사용자는 자동 탐색 정상 동작을 확인하고 실제 Add/Update/Remove 및 DCS 프로젝트 수신 구현을 요청했다. 이후 사용자 요청으로 기본 효과 5개를 추가했다. 효과 계약은 CONTRACTS.md 및 사용자 MANUAL.md를 따른다. 아래 값은 이 구현의 계약이며 실게임 검증 결과를 뜻하지 않는다.

## 파일과 설치

```text
<DCS profile>/Scripts/
  Export.lua
  StellaVibra Telemetry/
    loader.lua
    telemetry.lua
    config.lua
    manifest.json

```

Export.lua에 추가하는 관리 행:

```lua
pcall(dofile, lfs.writedir() .. [[Scripts/StellaVibra Telemetry/loader.lua]]) -- StellaVibra Telemetry
```

loader.lua는 telemetry.lua를 보호 호출한다. 중복 로드 방지는 Lua VM 전역 플래그다. telemetry.lua는 Start/OnHumanStart/OnHumanStop/AfterNextFrame/Stop 콜백을 체이닝한다. 기존 콜백을 먼저 보호 호출하고 실패를 한 번 기록한 후 Stella 콜백을 실행한다. ActivityNextEvent의 타 exporter 스케줄을 변경하지 않는다. 앞선 Export.lua 최상위 사용자 코드가 오류로 중단되거나, 뒤 exporter가 콜백을 체이닝 없이 덮어쓰면 Stella까지 실행된다는 보장은 없다.

config.lua는 관리 주석/return 형식 전체를 직접 파싱하고 정수10–100 범위만 허용한다. 숫자만 수동 변경하면 손상으로 보지 않고 설정 창에 실제 config 값을 표시한다. LF/CRLF는 허용하며 Lua를 실행하지 않는다. 형식/범위가 틀리면 Restore 대상으로 본다. config.lua의 기존 manifest 해시는 무시하고 새 manifest에는 loader.lua/telemetry.lua 해시만 저장한다. 변경 중 충돌 감지용 fingerprint에는 config의 현재 내용도 포함한다. 반환 코드는 `return { interval_ms = 50 }` 형식이다. 기본50ms, 정수10–100ms. 10ms는 과도한 직렬화 빈도를 제한하는 하한, 100ms는 현재100ms 지속 효과 요청과500ms 입력 만료를 고려한 상한이다. 게임 프레임이 느리면 설정 주기를 달성할 수 없으며 누적 송신을 따라잡기 위해 burst를 보내지 않는다. 100ms는 지속 효과 요청 갱신 여유가 없으므로 기본50ms를 권장한다. 파형 주기/HID 틱과는 별개다.

코드·주기 변경은 DCS 재시작 후 적용한다. 매 프레임 config 파일을 다시 읽거나 로더를 재등록하지 않는다. 코드 버전2/프로토콜1/설치 manifest schema2는 앱 버전0.2.1과 별개다. 구 schema1은 해시/주기를 읽어 호환하며 과거 Export.lua 원본 메타데이터는 무시하고 다음 저장 때 제거한다.

자동 탐색은 Windows Known Folder의 Saved Games를 사용한다. 프로필 하나는 자동 선택, 여러 후보는 발견 목록에서 선택한다. 창은 경로 → 주기(ms) → Add/Update/Restore Scripts → Remove Scripts → Done 순이다. 정상 상태는 툴팁, 조작 불가 오류는 경로 아래에 표시한다.

| 판정 | 버튼/동작 |
| --- | --- |
| Export.lua가 없거나 Stella 관련 행 없음 | Add Scripts 활성. 남은 관리 파일이 손상되었으면 추가 전 복구 확인 |
| 정확한 로더 한 행 + manifest 소유자/schema 및 코드 Lua 해시 일치 + config 전체 형식/범위 정상 | Update Scripts 활성(변경 없어도 활성). 번들과 버전/내용이 달라지면 최신 코드로 갱신 |
| Stella 언급 행이 표준과 다름·중복이거나 로더는 있는데 파일/manifest 누락·해시 불일치 | Restore Scripts 활성. 대상 행 상세를 제공하고 기본 No로 복구 확인 |
| Stella 관련 행 또는 전용 폴더 존재 | Remove Scripts 활성. 해시 불일치와 관계없이 삭제 확인 |

Stella 관련 행은 대소문자 무관 stellavibra 또는 stella vibra 문자열이 포함된 물리적 행이다. 복구는 첫 대상 행을 표준 로더로 교체하고 나머지 대상 행은 제거한다. 한 행에 다른 코드가 함께 있으면 그 행 전체가 바뀐다는 점을 확인창에 명시하고 대상 행을 상세 표시한다. 여러 행으로 분리된 임의 Lua 코드를 파싱·재작성하지 않는다. 복구는 알려진 loader.lua/telemetry.lua/config.lua/manifest.json만 교체하며 추가 파일은 그대로 둔다.

Export.lua 전체 디스크 백업·원본 저장·전체 복원은 제거했다. 현재 파일을 읽어 지정된 행만 편집하고 다른 바이트는 재인코딩하지 않는다. UTF-8 BOM·CRLF/LF/CR 및 끝 개행 없음은 보존한다. 추가할 때는 기존 첫 개행 스타일을 사용하고, 기존 끝에 개행이 없으면 구분 개행을 추가한다. 새 Export.lua는 BOM 없는 UTF-8/LF 한 줄이다. 제거 후 남은 바이트가 정확히0이면 Export.lua도 삭제한다. 공백/개행/BOM만 남아도0바이트가 아니므로 보존하며, 과거 추가했던 구분 개행을 되돌리지 않는다. ASCII 비호환 UTF16/32/NUL 파일 및 Export1MiB 초과는 거부한다.

확인창이 열린 사이 파일이 바뀌면 fingerprint 재검사로 작업을 중단한다. Export.lua는 관리 파일 쓰기 뒤 다시 동일함을 검사하고 필요한 경우에만 QSaveFile로 원자 교체한다. 다른 프로세스가 최종 확인과 교체 사이에 동시에 쓰는 극히 짧은 구간까지 잠그는 것은 아니므로 설치 작업 중 타 설치기의 동시 편집은 피해야 한다. 여러 파일 전체 원자 트랜잭션은 아니다. 실패 시 전용 관리 파일만 메모리 스냅샷에서 롤백하며 공유 Export.lua의 과거 내용을 복원하지 않는다. 중단/롤백 실패 후에는 상태 재검사와 Restore Scripts로 복구한다.

Remove Scripts는 확인 후 현재 Export.lua의 대상 행과 전용 StellaVibra Telemetry 폴더 내용 전체(수정 파일/추가 파일 포함)를 삭제한다. 폴더 전체 삭제 범위를 확인창에 명시한다. 실제 삭제 전 절대·canonical 경계 및 하위 링크/junction을 검사하며 링크 대상은 삭제하지 않는다. 다른 exporter 행, Scripts의 다른 폴더, 과거 버전이 남긴 백업 폴더는 건드리지 않는다. 부분 삭제 실패는 알리고 재시도할 수 있다.

## 통신

- DCS LuaSocket UDP → `127.0.0.1:43891`. 별도 중계 서버/상주 프로세스 없음. 수신기는 DCS 프로젝트가 활성화된 동안만 생성되며 다른 게임/프로젝트 닫기 때 소멸한다. 배타적 로컬 바인딩 실패는 로그와 재시도로 표시한다.
- UTF-8 JSON 객체 하나 = 데이터그램 하나, 최대16KiB. 프로토콜1. Lua는 제한된 구조만 선택하고 직렬화 깊이/항목 수/문자열 길이를 제한한다. 소켓 timeout0, 송신 주기가 된 프레임에서만 API 조회/직렬화한다.
- 필수 envelope: protocol, session, sequence, sim_time, sent_ms, interval_ms, stopped, dcs. session은 LuaSocket 시각·소켓 식별 문자열·생명주기 카운터 조합이며 인증 토큰은 아니다. sequence는 세션 내 증가한다.
- sent_ms는 같은 PC의 epoch ms이며 오래된 큐 패킷 판별에만 사용한다. 수신 시500ms 이상 지난 것,100ms 넘는 미래 값, 마지막 수락 송신 시각보다 뒤로 간 것은 버린다. 시스템 시계가 크게 바뀌면 재연결이 필요할 수 있다.
- 수신 시각은 호스트 monotonic 시각이다. sourceDeadline은 송신 때부터500ms에 해당하는 남은 수명으로 정하며, 같은 스냅샷의 내부 재전달은 원래 received/advancedAt/sourceDeadline을 갱신하지 않는다. 모델 시각 전진도500ms 안에 있어야 입력이 유효하다.
- 중복/역전 순번, 비로컬 송신자, 형식·버전 오류, 너무 큰 패킷은 버린다. 손실은 재전송하지 않는다. 최근 송신 소스 포트가 연결된 동안 다른 소스는 거부한다. 종료된 세션64개를 기억해 되돌림을 차단한다. 동시 DCS 여러 인스턴스 지원은 하지 않는다.
- Start/HumanStart는 새 세션, HumanStop/Stop은 stopped 패킷. 종료 패킷 유실/콜백 정지는 유효기간으로 처리한다. pause/active pause는 원본 플래그를 보내며 콜백 자체가 멈추면 수신 만료한다. ownship 권한이 true가 아니면 비행 데이터를 수집하지 않는다.
- 새 세션/기체 객체ID 변경/모델 시각 역행/500ms 이상 입력 공백은 상태 경계를 만든다. 첫 샘플은 기준 설정만 하며 두 번째 전진 샘플부터 유효하다. C++ 효과 인스턴스/요청은 경계에서 초기화한다. 실제 효과 이벤트 감지는 후속 Lua 책임이다.

## 원본 입력과 Lua 표면

`telemetry.dcs` 아래에 DCS API 이름을 그대로 둔다. `LoGetSelfData`는 Name만, `LoGetMechInfo`는 gear/flaps/speedbrakes, `LoGetEngineInfo`는 RPM만 선택한다. in-air, IAS/TAS/Mach/AoA, G벡터, 세계속도/각속도 벡터, shake, pause/권한/개체ID를 수집한다. relay v2에서 LoGetPayloadInfo.Cannon.shells와 Stations의 count/container/weapon.level1..4를 추가한다. station ID는 문자열 JSON키, 최대64개, 노드상한1024, payload포함16KiB초과면payload만생략. 조종석 인자/월드 객체는 수집하지 않는다.

`telemetry.transport`는 envelope 메타데이터, sample_id/received_ms/aircraft_key/sim_time_advanced는 Stella 호스트 메타데이터다. 누락/호출 오류/nil/비유한 숫자는 키를 생략하여 Lua에서 nil로 보인다. false/0은 그대로 보존한다. 원본 값을 다른 게임 필드로 변환하거나 rod를 검증된 압축 길이라고 표현하지 않는다. Telemetry 뷰와 자동완성은 선택한 원본 경로를 제공한다.

효과가 없는 DCS 프로젝트를 생성/열어도 수신은 동작한다. 새 DCS 프로젝트에서 기본 효과 생성을 선택하면 Ground Roll/Touchdown/Airframe Shake/G Load/Weapon Release를 복사한다. 게임별 효과와 출력 판단은 Lua에 있으며 C++에 추가하지 않았다. 기존 프로젝트는 덮어쓰지 않는다. 실기 검증은 사용자 담당이다.

## 확인 범위

API 근거는 설치 DCS 2.9.31.29434의 Scripts/Export.lua 및 기존 DCS-TELEMETRY.md 조사다. Lua5.1 호환 문법으로 작성했으나 빌드에 포함된 구문 검사는 Lua5.4 컴파일러다. 실제 DCS VM/기체별 반환·콜백 공존·UDP 수신·설치/제거 GUI는 사용자 확인 대상이며 자동 실행 테스트를 하지 않는다.
