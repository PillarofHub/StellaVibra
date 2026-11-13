# DCS 공통 텔레메트리와 효과 가능성 검토

조사일: 2026-11-13. 상태: 설계 검토, 구현 계약 미확정. 조사 시작 기준은 0.2.0이었다. 이후 파일 관리 UI 작업은 사용자 지정 0.2.1로 정리하며 DCS는 여전히 미구현이다.

## 후속 사용자 결정 및 엔진 검토

아래 초기 후보 평가보다 이 절의 범위 결정이 우선한다. MSFS 효과는 참고일 뿐 DCS의 입력/감촉에 맞춰 단순화한다. 기체별 프로젝트/전용 매핑을 기본 요건으로 두지 않는다.

| 효과 | 현재 방향 | 구현 의도 / 파라미터 제안 |
| --- | --- | --- |
| Ground Roll | 유지 | 전체 비공중+지상 이동 조건의 지속 진동. Power/최소 속도, 100ms 갱신. 노면 슬래브 모델 없음 |
| Touchdown | rod 기반으로 먼저 시도 | 기어별 rod 변화의 첫 상승 구간을 사용, 첫 샘플 기준 설정/재무장/누락 방지. Power/최소·최대 시간/배수는 출발안이며 MSFS와 일치시키기 위한 복잡한 보정은 추가하지 않음. 사용자가 실기에서 실패를 확인하면 통합 착지 펄스로 단순화. rod의 힘·압축 길이 단위가 확인된 것은 아님 |
| Airframe Shake | 유지, 진동 표현의 중심 | 원본 shake 크기에 따른 진동. Power/입력 감도·상한 등 최소 조절만 제안. 원인별 버핏/엔진 분리를 추정하지 않음 |
| G Load | 유지 | G에 따른 강도 조절. 시작 G/최대 강도 G/Power 제안. 양·음 G 정책은 후속 세부 논의 |
| Overspeed | 선택적 경고 효과로 유지 | 수동 속도/Mach 기준과 해제 여유값, Power. 기체별 기존 Override 사용. 물리적 속도 버핏의 재현이 아니라 사용자가 정하는 경고 |
| High AoA / 별도 Stall Buffet | 기본 효과에서 제외 | Shake와 감촉·역할 중복을 줄임. AoA 원본을 Lua 개발용으로 남기는 문제는 효과 제공 여부와 별개 |
| Ground Thumps | 제외 확정 | DCS 기본 Lua로 만들지 않음. 기존 타 게임의 Thumps는 변경하지 않음 |
| Engine RPM | 검토 후 보류 권고 | RPM에 선형 비례하는 진동은 만들지 않음. Shake가 충분한지 실기 확인 전 별도 효과/공명 모델을 기본 제공으로 확정하지 않음 |
| Startup/Shutdown | 미확정 유지 | 이번 사용자 메시지에서 별도 채택/삭제 결정 없음. 엔진 진동 검토와 함께 필요성 판단 |
| Weapon Release / Gunfire | 기존 후보 유지, 미확정 | Stick 기본 출력 희망 유지. 종류별 잔량 감소와 기총 burst, 비상투하/재무장 구분 한계는 이전 조사 그대로 |

엔진 추가 확인: 설치된 `Mods/aircraft/P-51D/FM/FMOptions.lua` 103–105행과 `FW-190D9/FM/FMOptions.lua` 83–85행에는 propBodyShakeK/propBodyShakeLim/propDisbalanceShakeLim이 있다. FM 제작자용 `ed_fm_get_shake_amplitude`의 공식 설명은 head simulation용 shake amplitude다. 이 둘은 엔진/프로펠러 흔들림이 모델링되는 근거지만, 최종 `LoGetShakeAmplitude()`에 그 성분이 반드시 모두 포함된다는 호출 경로는 공개 자료에서 확인하지 못했다. 제트/모든 기체로 일반화할 수 없다.

[TelemFFB 효과 문서](https://docs.vpforce.eu/telemffb/effects-mechanical/)는 DCS의 프로펠러/RPM rumble과 IL-2의 직접 engine-shake telemetry를 구분한다. 따라서 다른 게임의 전용 엔진 shake 지원을 DCS로 옮겨 설명하지 않는다. 동일 문서의 RPM 기반 rumble은 합성 효과 사례이며 실제 공명 응답을 제공한다는 증거가 아니다.

결론: 엔진 Shake 통합은 **일부 성분이 포함될 가능성은 있으나 전 기체 보장 미확인**. RPM%만으로 실제 공명 주파수/진폭을 계산할 근거도 없다. 별도 RPM 효과를 당장 구현하지 않고 Airframe Shake부터 평가하는 것이 단순한 권고안이다. 이는 엔진 진동 전체가 이미 Shake에 포함된다는 확정이 아니다. Overspeed도 버핏이 반드시 먼저 발생한다는 전제를 두지 않고 독립 사용자 경고로 둔다.

## 판단 기준

사용자 정정에 따라 효과 선택에 앞서 입력 제공 범위를 조사한다. 하나의 DCS 프로젝트/기본 Lua 구성을 목표로 하며, 주 사용 기체(호넷·머스탱·도라)를 지원 범위의 선택 기준으로 쓰지 않는다. 기체별 ID/조종석 인자 매핑을 전제로 공통 기능이라고 부르지 않는다.

이 문서에서 **확인됨**은 설치된 공식 참조 파일에 API/필드가 명시되었다는 뜻이다. 게임 실행으로 모든 기체의 값과 의미를 검증했다는 뜻이 아니다. 공통 API, 실제 반환값의 가용성, 효과의 해석 가능성을 따로 판단한다. 기체별 정상 0/false와 미구현 고정 0/false를 값만으로 구분할 수 있는 보장은 발견하지 못했다.

## 근거와 확인 범위

1. 설치 경로 `D:\Eagle Dynamics\DCS World`, `autoupdate.cfg`의 version `2.9.31.29434`.
2. 공식 설치 참조 `Scripts/Export.lua`: 콜백 26–130행, API 설명 424–862행, 최신 상태/FM 항목 1451–1469행. SHA256 `B0823AA1F239A58159BB1FAB6040341AD41CBC7EA33D526E171C8FEA878A695D`.
3. 공식 설치 `API/Sim_ControlAPI.md`: Hooks와 Export 환경 구분 17–29행, export 권한 485–551행, 별도 GUI Hooks 콜백 565–612행. SHA256 `FFFE36E26198A3EFF415D69EF04910E6C98AD953321E14C1615E2A9876B3A2E6`.
4. 공식 설치 `Scripts/Database/wsTypes.lua`: 무장 분류. `API/include/FM/wHumanCustomPhysicsAPI.h`: FM 제작자용 shake/suspension 인터페이스. 이것은 일반 exporter가 타 모듈의 FM 구조체를 읽을 수 있다는 근거가 아니다.
5. 공식 설치 `Mods/aircraft/FA-18C/FM/config.lua` 및 `AH-64D/FM/config.lua`: `arg_amortizer` 1/6/4의 실제 사용 사례. 머스탱/도라의 공개 FM Lua에서는 같은 매핑의 근거를 찾지 못했다. 부재를 미지원으로 단정하지 않는다.
6. [ED 공식 변경 기록](https://www.digitalcombatsimulator.com/en/news/changelog/): 2026-09-30에 player seat, model time acceleration, network/track 상태, FM master 설명 추가. 구버전에서도 모두 있다고 가정하지 않는다.
7. [TelemFFB 실제 exporter](https://github.com/walmis/VPforce-TelemFFB/blob/master/export/TelemFFB.lua): 공통 조회와 기체별 예외를 구분하기 위한 비교 자료. 2026-11-13 master 조회, 고정 커밋 자료는 아님. 코드 복사/의존성 추가 없음.

사용자 Saved Games/Export.lua는 조회·설치·수정하지 않았고, DCS·앱·HID·프로브를 실행하지 않았다. 실제 샘플·비용 계측 결과는 없다. 설치 문서도 API가 변경/제거될 수 있다고 명시하므로 함수 존재와 반환 자료형을 확인하는 구현이 필요하다.

## 일반 Export 입력

| 항목 | 실제 API/필드 | 문서상 의미·단위 | 공통 효과에 사용할 때의 한계 |
| --- | --- | --- | --- |
| 기체·개체 | `LoGetSelfData()`의 `Name`, `Type`, `UnitName`, `Position`; `LoGetPlayerPlaneId()` | 기체/개체 정보, 객체 ID | 기체 종류와 같은 기종의 다른 개체를 구분. 전역적으로 유일한 세션 UUID는 아님 |
| 시각 | `LoGetModelTime()`, `LoGetMissionStartTime()` | 초 | 반복 임무/시간 역행 가능. 수신 벽시계와 분리 |
| pause | `LoSimulationOnPause()`, `LoSimulationOnActivePause()` | boolean | API 확인됨. pause 중 콜백 실행 보장은 별도이며 수신 만료 필요 |
| 공중 여부 | `LoGetInAir()` | 자기 객체가 공중인지 boolean | 기체 전체 상태. 바퀴별 접촉, 하중 또는 압축 아님. false를 개별 바퀴의 WOW로 복제하지 않음 |
| 속도 | `LoGetTrueAirSpeed()`, `LoGetIndicatedAirSpeed()`, `LoGetMachNumber()` | TAS m/s, Mach 무차원. 설치 문서는 IAS 단위 생략; 실제 exporter는 m/s로 사용 | IAS는 외부 구현 근거와 공식 문서의 빈 부분을 구분. 자동 Vne/Mmo 입력은 확인 못함 |
| 세계 속도 | `LoGetVectorVelocity()` | 세계 축 x/y/z 벡터 | 해당 항목의 단위 주석은 생략. 외부 구현은 풍속과 함께 속도 벡터로 계산. 지상 수평 속도는 Lua 파생값이며 항모 갑판 상대속도와 다름 |
| 자세 | `LoGetADIPitchBankYaw()` | rad | 계기/기체 자세 의미 유지 |
| G | `LoGetAccelerationUnits()` | x/y/z, G | 이 문서만으로 축 방향·중력 포함 세부를 모두 확정하지 않음. 수직 하중 해석 확인 후 효과에 사용 |
| 받음각 | `LoGetAngleOfAttack()` | rad | 공통 조회 가능. 실속 임계각·버핏 시작각은 함께 주어지지 않음 |
| 수직 속도·고도 | `LoGetVerticalVelocity()`, ASL/AGL, `LoGetHeightWithObjects()` | m/s, m | 기어 충격·지면/갑판 상대 하강률과 동일하지 않음 |
| 진동 크기 | `LoGetShakeAmplitude()` | shake/vibration amplitude, 숫자 | 범위·정규화·원인별 분리 미명시. 단위 없는 원본 스칼라로 보존, 0..1 강제 가정 금지 |
| 엔진 | `LoGetEngineInfo().RPM.left/right`, `Temperature`, `FuelConsumption`, `HydraulicPressure` | RPM %, 온도 섭씨, 연료유량 kg/s, 압력 kg/cm² | 좌/우 스키마. 단발의 두 값/엔진 수를 추정하지 않음. 추력·토크·공통 starter/combustion은 문서 구조에 없음 |
| 기계 상태 | `LoGetMechInfo().gear`, `flaps`, `speedbrakes` 등의 `status/value` | 필드 확인됨, 각 상태 코드·값 범위는 이 문서에 미명시 | 값 변화는 관측 가능하지만 모두 0..1/퍼센트라는 계약은 아직 불가 |
| 기어 rod | `gear.main.left/right/nose.rod` | 필드 이름만 명시 | 압축 방향·단위·정규화·기체별 갱신 보장 불명. 현재 바퀴별 압축 계약으로 확정 불가 |
| 실속 경고 | `LoGetMCPState().StallSignalization` | boolean | 필드는 확인됨. 모든 모듈의 고유 실속 경고를 공통 반영한다는 보장 없음 |
| 무장 | `LoGetPayloadInfo().Stations`, `.CurrentStation`, `.Cannon.shells` | station별 `weapon.level1..4`, `count`, `container`; 기총 잔탄 | 재고 스냅샷. 발사·비상투하·재무장 이벤트가 아님. `CurrentStation`은 발사 증명 아님 |
| 보조 FM | `LoGetFMData()` | cockpit G_factor, body-axis speed/acceleration, angular speed rad/s, angular acceleration rad/s², 자세 rad, master | 세부 필드 가용성·일부 단위 미명시. 기존 속도/G와 중복 수집하지 말고 필요 시 선택 |

## 압축·접지에 관한 결론

일반 Export에 공중 여부와 rod 필드가 있으므로 “DCS는 접지/압축을 주지 않는다”는 결론은 잘못이다. 그러나 rod의 물리적 의미와 범위를 문서만으로 확정할 수 없어 “공통 압축이 확인됐다”도 아직 성립하지 않는다.

`LoGetAircraftDrawArgumentValue()`는 외형 애니메이션 인자를 읽는 API다. 조종석 `GetDevice(0):get_argument_value()`와 다른 공간이다. 호넷·아파치의 설치 FM 설정은 압축 인자 1/6/4를 사용한다. 외부 TelemFFB는 이 인자를 기본값으로 읽지만 UH-1H/OH-6A에는 다른 인자 처리가 있다. 따라서 모든 DCS 기체의 바퀴별 압축으로 단정하지 않는다. 특히 tailwheel/skid/multiple bogie를 nose/left/right 3개로 고정하는 데 한계가 있다.

공통 프로젝트의 Ground Roll은 전체 in-air 상태와 속도로 구성할 수 있다. 이는 통합 Ground 출력이며, 기존 MSFS의 바퀴별 접촉과는 다르다. 지상 속도는 세계 기준이라 정지한 항모 위에서도 배의 이동이 포함될 수 있다. 확인된 갑판 상대속도 없이 항모에서 완전히 같은 사용감을 보장하지 않는다.

Touchdown은 공중→비공중 전환 자체로 구현할 수 있다. 다만 기존 첫 압축 상승 방식은 rod/외형 인자의 의미 확인 전에는 유지한다고 약속할 수 없다. 공통 기본안은 고정 길이의 통합 착지 펄스가 가능하고, 압축 기반은 보류다. 수직 속도/G로 지속 시간을 결정하는 대안은 새로운 착륙 모델이며 첫 압축 상승의 대체 동등 구현이 아니다. 이 변경은 사용자 합의 전이다.

외부 FM 헤더의 `ed_fm_suspension_info.struct_compression`, `acting_force`, `wheel_speed_X`는 FM 제작자에게 전달하는 구조다. 이것을 Export Lua의 제공 필드로 넣지 않는다.

## 효과별 가능성과 타협안

| 효과 | 공통 구현 가능성 | 타협/권장 범위 |
| --- | --- | --- |
| Ground Roll | 전체 상태 기준 가능 | 전체 비공중+속도 조건. 바퀴별 분리와 갑판 상대속도는 보장 제외. UI Power/최소 속도 및 100ms 갱신 의도 유지 가능 |
| Touchdown | 전체 착지 전환 가능, 첫 압축 상승은 미확정 | 접지 전환 기반 고정 펄스 또는 압축 확인 때까지 보류. min/max/multiplier를 고정 펄스에 무의미하게 노출하지 않음 |
| Ground Thumps | 지상 거리 기반 합성 펄스 가능 | 표준 Export 목록에서 포장 종류/활주로 여부/실제 이음매를 확인 못함. 범용 가상 지상 요철로 의미를 낮추거나 기본 제공 보류. Terrain/mission/EFM API를 Export에서 쓸 수 있다고 추정하지 않음 |
| Airframe Shake | 공통 amplitude 입력으로 가능 | 실속·엔진·노면 원인 분리 없는 DCS shake 반응. gain/deadzone/full-scale 조정이 필요하고 실제 FFB 힘은 아님 |
| High AoA | 가능 | 사용자 임계각 기반 진동. 전 기체 공통의 정확한 실속 임계값은 없음. 같은 Lua에 파라미터 조정 허용, 기종 분기 자동값은 넣지 않음 |
| Stall/Buffet | 정확한 공통 실속 버핏으로는 미확정 | Airframe Shake와 High AoA로 의미 분리 권장. MCP 실속 값은 선택 원본 입력으로만 고려 |
| G Load | G 입력 기반 가능 | 축 의미 확인 및 양/음 G 처리 필요. 관측 하중 알림이지 힘/저항 재현 아님 |
| Airspeed | 속도 기반 가능 | TAS/IAS/Mach 조건과 수동 기준. 공통 자동 과속 한계를 제공한다고 표현하지 않음 |
| Engine RPM | 가능 | 원본 left/right RPM% 기반 출력. 엔진 추력 효과로 부르지 않음. 단발의 복제 값이나 없는 엔진을 임의 생성하지 않음 |
| Startup/Shutdown | 회전 전환 추정으로 가능 | RPM 상승/하강과 유지 시간·히스테리시스 사용. 실제 점화/연소 확인 아님. 윈드밀링·재시동·flameout 구분 한계. 정확한 공통 시동 이벤트는 미확인 |
| Weapon Release | 잔량 감소 기반으로 가능 | 폭탄/미사일/로켓 종류별 이탈 펄스. 정상 발사와 비상투하를 구분하지 못하므로 Launch 확정 표현 금지 |
| Gunfire | 제공되는 공통 잔탄 감소로 추정 가능 | 발사 burst 진동으로 표현, 개별 발사 시각 복원 불가. 무한탄약/미갱신/일부 모듈 탄수 표현 차이에서는 미동작 가능 |
| Flaps/Speedbrakes | 원본 위치 변화/전개 신호로 조건부 가능 | 원본 status/value 의미·범위 확인 전 고정 퍼센트 기준을 넣지 않음. 기체별 인자 보정은 기본 공통 범위 밖 |

## 무장 스냅샷의 구분 가능 범위

공식 wsTypes는 level1=Weapon(4), level2=Missile(4)/Bomb(5)/Shell(6)/NURS(7)를 구분한다. 이는 종류 분류이며 발사 이벤트 증명은 아니다. 문자열 이름으로 미사일/폭탄을 추정할 필요는 없다.

동일 개체·동일 station·동일 weapon type의 수량 감소만 후보로 삼고, 첫 샘플·시간 역행·큰 공백·기체 교체·종류 변경·수량 증가에는 기준을 다시 잡으면 명백한 오검출을 줄일 수 있다. 그러나 같은 종류를 제거하는 재무장 단계와 비상투하는 이 규칙만으로 완전히 구분되지 않는다. station 소멸을 0으로 대입하면 unavailable을 발사로 오인하므로 금지한다. 모듈이 소멸로만 이탈을 알린다면 보수적 구현은 그 이벤트를 놓칠 수 있다.

정확한 발사 이벤트를 위해 mission world 이벤트나 GUI Hook을 사용하는 것은 별도 실행 환경/권한/설치 범위다. 공식 Sim_ControlAPI의 Hooks는 Scripts/Hooks에 별도 등록하며 문서상 onGameEvent 목록에 일반 shot 이벤트가 없다. Export 콜백만으로 mission 이벤트가 자동 제공된다고 가정하지 않는다. 현재 요구에 맞추어 공통 기본 exporter에 미션 삽입이나 추가 Hook을 요구하지 않는 방향을 권장한다.

## 공통성·가용성 경계

- 하나의 DCS 프로젝트, 같은 Lua와 원본 입력 구조를 사용한다. 기체 이름 switch나 조종석 인자 표를 기본 효과의 필수 기반으로 쓰지 않는다.
- 이름/객체 ID는 인스턴스 상태 초기화와 기존 기체별 파라미터 상속에 쓰며, 특정 기체만 지원하기 위한 분기가 아니다.
- 공통 함수가 반환하지 않는 필드, nil/비유한 수, 권한 거부, 호출 실패는 unavailable. 정상 0/false는 보존한다. 값이 고정됐다는 이유만으로 미지원이라고 단정하지 않는다.
- 실제로 지원하지만 현재 0인 경우와 미구현으로 0인 경우를 완전히 구분할 수 없는 한계도 표시한다. 예: 미사용 draw argument의 0을 압축 0의 증거로 삼지 않는다.
- 기체별 예외를 추가하지 않는 선택은 일부 모듈에서 효과가 작동하지 않거나 통합 출력만 가능하다는 제약을 수반한다. 전 기체 동일 동작 보장은 아님.

## 상태와 멀티플레이

공식 Export에는 Start/Stop, OnHumanStart/OnHumanStop 및 모델 시간이 있다. 연결 첫 샘플·개체 ID 변경·새 인간 기체·시간 역행에서 효과 기준을 다시 잡을 수 있다. 같은 종류의 재스폰도 초기화해야 한다. 세션 식별자는 중계기 생명주기에 맞춰 생성해야 하며 mission start time만으로 유일성을 주장하지 않는다.

pause와 active pause는 둘 다 API가 있으나 pause 동안 exporter가 스케줄되지 않으면 상태 패킷도 못 보낼 수 있다. 마지막 수신 시각과 모델 시간 전진의 만료는 필요하다. 종료 콜백/패킷이 누락되더라도 호스트가 오래된 입력을 계속 실행하지 않게 해야 한다. 구체 유효기간은 전달 계약 단계에서 정한다.

멀티플레이 ownship export 허용 여부가 주요 입력을 제한한다. 공식 Sim_ControlAPI는 SelfData까지 ownship 제한 목록에 넣으며 Export.lua의 오래된 SelfData 설명과 표현 차이가 있다. 더 보수적으로 현재 ownship 허용 상태를 존중하고 우회하지 않는다. world object/sensor 권한은 별개이며 이번 효과 때문에 모든 월드 객체나 표적을 수집할 이유는 없다.

## 수집 우선순위와 비용 제안

| 묶음 | 제안 주기 | 이유·비용·제약 |
| --- | --- | --- |
| 시각/pause/권한/개체 식별/공중 여부 | 송신마다, 생명주기 콜백 보조 | 상태 전환 누락 방지. 소수 스칼라/작은 객체. 기체 이름만 별도 느린 수집하면 잘못된 상태 유지 위험 |
| 속도/AoA/G/shake/수직 속도 | 기본 20Hz | 지속 효과에 유의미. 소수 값, 게임 프레임보다 빠른 새 관측은 불가 |
| RPM 및 합의한 기계 상태 | 기본 20Hz | 작은 테이블을 한 번 조회하고 필요한 필드만 선정. 같은 API를 필드마다 반복 호출하지 않음 |
| 무장 station/기총 잔탄 | 기본 20Hz | station 수에 비례한 조회/직렬화 비용. 짧은 burst를 잔량 변화로 감지하지만 개별 발사 시간은 손실 |
| rod/외형 인자 | rod는 의미 확인 목적의 선택 원본 후보; draw 인자 기본 수집 제외 | 의미 미확정 값으로 기본 효과를 구동하지 않음. 모든 draw/cockpit 인자 스캔 금지 |
| 버전/기타 정적 설명 | 시작·연결 때 | 반복 직렬화 불필요. 무장 이름은 종류 변경 시 캐시 가능 |
| 전체 월드/표적/라우트/조종석 전체/중복 FM | 기본 제외 | 현재 효과에 비해 비용·권한·기체 의존성이 큼 |

20Hz=50ms는 샘플 간격이지 파형 주파수나 펄스 길이가 아니다. 100ms 지속 요청은 정상 간격 두 번 정도를 덮지만 프레임 저하/손실에서 공백이 가능하다. HID 기본20ms와 수신 만료는 별도다. 50ms보다 짧은 압축 첫 상승은 샘플 사이에 사라질 수 있고 FPS를 올려도 게임 프레임 이상의 관측은 불가하다. 이벤트 효과는 관측 시점에 별도 기한을 시작하고 동일 스냅샷 재전달로 이벤트/수신 시각을 새로 만들지 않는다.

비용은 반환 구조에 따른 정성 평가이며 호출 시간/패킷 바이트/게임 FPS 영향을 계측하지 않았다. 포트·프로토콜·최대 메시지·FPS 범위는 아직 정하지 않았다.

## 다음 논의의 근거

권장 공통 후보는 Ground Roll(통합), Airframe Shake, High AoA, G Load, Airspeed, Engine RPM이다. Touchdown은 통합 고정 펄스 타협 또는 압축 확인 전 보류, 시동/정지는 RPM 추정, 무장은 종류별 잔량 감소/기총 burst 추정이라는 제약을 먼저 명시한다. Ground Thumps는 포장 활주로 전용을 포기한 합성 효과로 바꾸지 않는 한 보류한다. 이것은 구현 확정이 아니라 입력 조사에 근거한 제안이다.

남은 불확실성은 공통 rod의 의미/범위·갱신 보장, G 축/세계 속도 단위의 더 명시적인 근거, shake 스케일, 실제 반환 지원의 기체별 차이다. 정적 자료만으로 해소되지 않은 부분을 사용자에게 이미 검증된 입력인 것처럼 넘기지 않는다. 특정 기체별 프로젝트/보정을 전제로 해결하지 않는다.
