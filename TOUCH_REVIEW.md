# 터치 감지 / 모터 구동 파형 검토 결과 (2026-09-08)

대상: `P_Wistec_HairBrush_V10` (main.c, Mode0)
검토 자료: `wave/digital.csv`(OUT1/OUT2, 로직 애널라이저), `wave/analog.csv`(터치 ADC, P10)

---

## 1. 채널/핀 매핑

| 캡처 채널 | 펌웨어 핀 | 정의 위치 | 역할 |
|---|---|---|---|
| digital ch5 | `P_O_OUT1` (P20) | [sys_const.h:47](sys_const.h#L47) | Q21 이후, H-브릿지 구동 |
| digital ch6 | `P_O_OUT2` (P21) | [sys_const.h:48](sys_const.h#L48) | Q22 이후, H-브릿지 구동 |
| digital ch4(초기 캡처) / ch7(라벨) | `P_O_MOTORCTRL` (P13) | [sys_const.h:41](sys_const.h#L41) | 모터 전체 On/Off 인에이블 |
| analog ch4 | `P_I_TOUCHADC` (P10) | [sys_const.h:38](sys_const.h#L38) | 터치 센서 ADC 입력 |

OUT1/OUT2는 [main.c:777-808](main.c#L777-L808) `TMR2_IRQHandler`의 `C_WISTEC_MODE0` 분기가 구동. MOTORCTRL은 [main.c:598-611](main.c#L598-L611) `TMR1_IRQHandler`가 `F_MOTOR_ON` 값에 따라 게이팅.

---

## 2. 정상 동작 검증 (파형-코드 일치 확인됨)

- OUT1/OUT2 교대 토글 주기 실측 **~24.98ms** (반주기) ↔ 코드 `Cnt10usec >= 50` (500µs×50=25ms, [main.c:786](main.c#L786)) — 오차 0.08% 이내, **일치**.
- 약 1000ms마다 반주기가 ~75ms(50+25ms)로 늘어나는 패턴 **다수 확인** → `TOUCH_CHECK_PERIOD_MS=1000` / `TOUCH_CHECK_WINDOW_MS=25` ([main.c:1713-1714](main.c#L1713-L1714))와 일치, **터치 체크 윈도우가 캡처 전 구간(36초)에서 정상적으로 계속 발동**하고 있음을 확인.

---

## 3. 발견 및 수정한 버그 — 터치 해제 판정 데드존

**증상**: 손을 뗀 뒤(no-touch)에도 모터가 계속 구동됨 (digital.csv에서 OUT1/OUT2가 11초 이상 끊김 없이 계속 토글).

**원인**: [main.c:1746-1758](main.c#L1746-L1758) (수정 전)
```c
if ( rTouchADC > (4095*2.0)/5 )        // 2V 초과 → Not Connect 처리
{ ... F_MOTOR_ON = FALSE ... }
else if ( rTouchADC > (4095*1)/5 )     // 1V~2V, 주석은 "Not Connect"지만
{
    // 빈 블록 — 아무 동작 없음!
}
else                                    // 1V 미만 → Connect
{ ... F_MOTOR_ON = TRUE ... }
```
실측 미터치 전압(~1.65~1.69V)이 이 **1V~2V 데드존**에 해당해서 상위(2V) 임계값을 넘지 못해 `F_MOTOR_ON`이 절대 FALSE로 바뀌지 않음 → 모터가 꺼지지 않음.

**수정** ([main.c:1744-1755](main.c#L1744-L1755)): 데드존을 없애고 1V 초과는 모두 "Not Connect"로 합침.
```c
if ( rTouchADC > (4095 * 1) / 5 )		// Not Connect
{
    if ( ++rTouchChattering > 25 )
    {
        rTouchChattering = 23;
        F_MOTOR_ON = FALSE;
    }
}
else						// Connect
{
    if ( --rTouchChattering == 0 )
    {
        rTouchChattering = 15;
        F_MOTOR_ON = TRUE;
    }
}
```
→ **적용 완료.**

---

## 4. 미해결 이슈 — 터치 ADC가 모터 스위칭 노이즈에 오염됨 (하드웨어 추정)

실제 손가락 터치 상태 캡처(analog.csv)를 ms 단위로 확대 분석한 결과:

- 100ms 단위로 볼 때는 "~1초 주기로 0.85V↔2.2V를 오간다"처럼 보였으나, **이는 5ms 단위 재샘플링 결과 에일리어싱(착시)으로 판명**됨.
- 실제 신호는 **~50ms 주기(20Hz)로 0.4V~2.4V 사이를 계속 진동** — OUT1/OUT2 모터 구동 주파수(20Hz)와 정확히 동일.
- 즉 터치 여부와 무관하게, **모터가 도는 동안 터치 ADC 값이 거의 항상 모터 스위칭 노이즈에 절어 있음**.
- 코드는 이를 피하기 위해 25ms 터치 체크 윈도우 동안 OUT1/OUT2를 스위칭 없이 고정(둘 다 HIGH)시키도록 설계돼 있음([main.c:779-784](main.c#L779-L784))인데, 실측 데이터상 해당 구간으로 추정되는 시점(t=14.122~14.172s)에서도 OUT1이 0으로 남아있고 ADC 값도 여전히 크게 흔들리는 것으로 관측됨 — **윈도우가 의도대로 "조용한" 구간을 만들어주지 못하고 있을 가능성**.

**결론**: 소프트웨어 임계값 조정만으로는 근본 해결이 어려울 수 있음. 터치 센스 라인(P10)과 모터 구동 라인(OUT1/OUT2, Q21/Q22) 사이의 커플링/공통 임피던스, 또는 프로브 접지 등 **하드웨어 신호 무결성 문제**로 추정됨.

### 다음 확인 필요 사항
1. 회로도/PCB에서 터치 ADC 입력(P10) 라인이 모터 구동부와 얼마나 가깝게 지나가는지, 공통 그라운드/전원 임피던스를 공유하는지 확인
2. 터치 체크 윈도우(OUT1=OUT2=HIGH 고정) 구간에서 실제로 스위칭 노이즈가 사라지는지 정밀 재측정 (측정 환경/프로브 접지 재점검 포함)
3. 필요 시 터치 ADC 라인에 RC 로우패스 필터 추가, 또는 ADC 샘플링을 윈도우 중반 이후로 늦춰서 과도현상 회피 검토
