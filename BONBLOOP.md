# BONBLOOP

**BONBLOOP** é um firmware para o **M-VAVE FM-1**, feito a partir do [SLOOP 2.8.5](https://github.com/isod89/sloop-fm1)
(que por sua vez é baseado no [Felucca](https://github.com/hugelton/Felucca)). Ele é **idêntico ao SLOOP** —
mesmos 4 tracks, sequenciador, bateria, efeitos, projetos, editor web — e acrescenta o **MODO SYNTH**: um
sintetizador **wavetable** com página própria, envelope, LFO e efeitos.

Licença: GPL-3.0-only, como o SLOOP e o Felucca (ver `LICENSE` e `LICENSING.md`).

---

## O MODO SYNTH

### Entrar e sair

| Gesto | O que faz |
| --- | --- |
| **Segure ENV ~0,7 s** | Liga o MODO SYNTH. A tela mostra **SYNTH MODE** e abre a página do wavetable. |
| **Segure ENV de novo** | Desliga e volta ao SLOOP normal (página ENV). O som do track continua. |

Ao ligar:

- o **track selecionado** passa a tocar o engine **WAVE** (o primeiro som dele, *WT BASIC*), a menos que já
  esteja em WAVE — aí o som é mantido;
- se a **bateria** (track 4) estiver selecionada, o modo pega o **track 1**;
- o cabeçalho mostra o selo **SYNTH** na cor do track e a linha sob ele fica colorida.

Um toque rápido em ENV continua abrindo a página ENV, como sempre; só o gesto de **segurar** liga o modo.

### Os botões dentro do modo

Dentro do MODO SYNTH, quatro botões abrem as páginas do synth em vez das suas próprias. Tocar de novo no mesmo
botão passa para a próxima página do grupo; o **SELECT** percorre todas as 13 páginas.

| Botão | Páginas | KNOB 1 · KNOB 2 · KNOB 3 · KNOB 4 |
| --- | --- | --- |
| **EDIT** | WAVE | TABL · POS · WARP · SUB |
| | OSC | DTN · VCE (POLY/MONO/LEG/UNI) · DTUNE (abertura do unison) · GLD |
| | FILTER | CUT · RES · DRV · FLT (envelope → filtro) |
| **ENV** | ENV | ATK · DEC · SUS · REL |
| | ENV DEST | FLT · PIT · **POS** · SYNC |
| **LFO** | LFO | RATE · WAVE · PHS · FADE |
| | LFO 2 | SYNC · TRIG · POL |
| | LFO DEST | PIT · FLT · **POS** · AMP |
| **FX** | FX | DST · CHO · DLY · REV |
| | INSERT / INSERT 2 | o INSERT do track (SOFT, HARD, FOLD, FUZZ, CRUSH, PHASR, FLANG, CHOR) e o filtro DJ do track |
| | DLY / REV/CHO | o delay e o reverb/chorus globais |

Por enquanto ENV, LFO e FX são exatamente os do SLOOP — a novidade é que, no engine WAVE, os destinos
**SHP** do envelope e do LFO aparecem como **POS** e **varrem o wavetable**: é o clássico *wavetable sweep*.

- **PRESETS** (o encoder), numa página do synth: passa pelos sons WAVE.
- **HOME**: vai para a tela TRACKS; o modo continua ligado (o selo SYNTH fica no cabeçalho) e ENV/LFO/FX/EDIT
  voltam às páginas do synth.
- Teclado, sequenciador, REC, as camadas (segurar FX, SEQ, GLO…) — tudo funciona como no SLOOP.
- O modo é só da interface: nada novo é gravado. O som é do track, como qualquer outro (salve em SAVE > USER).

### A tela do wavetable

A página **WAVE** (e OSC) desenha os **8 frames** da tabela, um atrás do outro (cinza), e na frente, na cor do
track, a forma de onda **na posição que está tocando**, entre dois frames. Com uma nota soando, a posição
inclui a modulação (LFO/ENV → POS), então dá para ver o wavetable se mexer.

---

## O engine WAVE

Um oscilador wavetable de 8 frames × 256 amostras, lido com interpolação entre amostras e entre frames.

| Parâmetro | O que faz |
| --- | --- |
| **TABL** | A tabela (abaixo). |
| **POS** | Onde na tabela ele lê: do frame 1 ao 8. ENV/LFO → POS somam a isso. |
| **WARP** | Dobra a fase de leitura: a primeira metade do ciclo é espremida (mais brilho, um "PWM" em qualquer tabela). O DC que isso criaria é compensado. |
| **SUB** | Uma senoide uma oitava abaixo. |
| **CUT / RES** | Filtro passa-baixa (o SVF do SLOOP), com 50 % de key tracking; ENV/LFO → FLT e o CC 74 movem o CUT. Aberto e sem RES ele sai do caminho. |
| **DTN** | Um segundo oscilador na mesma tabela, DTN cents acima (0 = desligado). |
| **DRV** | Saturação antes do filtro. |

### As 10 tabelas

| Tabela | Do frame 1 ao 8 |
| --- | --- |
| **BASIC** | seno → triângulo → dente de serra → quadrada → pulso 25 % |
| **PWM** | pulso de 50 % até 5 % |
| **HARM** | órgão: um harmônico a mais a cada frame |
| **RESO** | um pico ressonante subindo por uma dente de serra |
| **VOCAL** | vogais A, A-E, E, I, I-O, O, U, ER |
| **DIGI** | espectros digitais estilo PPG |
| **FOLD** | uma senoide dobrada (wavefolder) cada vez mais forte |
| **FM** | seno modulado a 3×, índice subindo |
| **BELL** | parciais esparsos, vítreos |
| **CHIP** | pulsos e degraus de 4 bits (com aliasing de propósito) |

As tabelas aditivas vão até o 24º harmônico (limpas até notas de ~900 Hz; acima disso dobram suavemente, como
os wavetables de 8 bits dos anos 80).

### Sem gastar flash nem RAM

O flash do FM-1 está quase cheio, então as formas de onda **não ficam no flash**: a tabela escolhida é
**calculada** quando você a escolhe, dentro da "arena" que os engines GRAIN/FM6/PHYS já compartilham por track
(nenhum byte de RAM a mais; um `_Static_assert` garante isso). A construção é feita aos poucos no áudio
(4 harmônicos por bloco: ~35 ms por tabela) e substitui a tabela que está tocando frame a frame — trocar
TABL nunca deixa o som mudo. O custo de CPU de uma voz é o mesmo de uma voz ANALOG.

Para ganhar espaço no flash, o logo de boot é o texto **BON / BLOOP** em vez da imagem do SLOOP
(`FELUCCA_LOGO_TEXT=1`, ~4,6 KB a menos). Medido no pacote oficial do SLOOP 2.8.5: 571.820 de 581.564 bytes
usados (9,7 KB livres) com o logo em imagem; com o logo em texto sobram ~14 KB para o WAVE e o MODO SYNTH.

### Os 15 sons

WT BASIC, WT SWEEP, PPG PAD, VOX SWEEP, WT CHOIR, PWM STRNG, WT ORGAN, WT PLUCK, WT BELLS, RESO BASS,
DIGI BASS, FM GROWL, FOLD LEAD, SUPER WAVE, CHIP WAVE — no navegador de PRESETS por categoria (BASS, ORGN,
PAD, LEAD, PLCK), com volume equalizado aos do ANALOG (`WAVE_TRIM`).

---

## Compatibilidade

- Projetos, presets de usuário, samples e ajustes do SLOOP continuam valendo.
- Um projeto com um track em WAVE (engine 12), aberto num SLOOP oficial, cai no engine padrão daquele track
  (o SLOOP não conhece o engine 12).
- O editor web lê a lista de engines do próprio aparelho, então o WAVE aparece nele.
- A identidade do pacote continua `FM-1_900` (build de desenvolvimento), instalável pelo mesmo instalador.

## Compilar

Igual ao SLOOP (ver [BUILDING.md](BUILDING.md)): toolchain JieLi (Linux x86-64, via WSL ou Docker) e três
arquivos do SDK AC79.

```
./build.sh                      # build/felucca.fwsc
./build.sh --release 1.0        # build/felucca-1.0.fwsc, versão "BONBLOOP 1.0"
```

## Testes

`tests/run_tests.sh` roda tudo, incluindo os dois testes novos:

- `tests/wave_test.c` — o engine sozinho: todas as tabelas, todos os presets (limites, DC, volume), extremos,
  custo de CPU; grava `wave_presets.wav` e `wave_sweep.wav` para ouvir.
- `tests/synth_mode_test.c` — a interface real: segurar ENV, os botões e as páginas do synth, SELECT, PRESETS,
  HOME, sair, a bateria selecionada; grava as telas em `synth-*.ppm`.

A suíte de regressão (`tests/regress.c`) confirma que **nenhum som do SLOOP mudou**: as 182 renderizações
anteriores batem bit a bit com o `golden.txt`; as 15 do WAVE foram acrescentadas.

## Créditos

- **SLOOP** — isod89 e colaboradores (GPL-3.0).
- **Felucca** — Leo Kuroshita / Hügelton Instruments (GPL-3.0).
- **BONBLOOP** — o MODO SYNTH e o engine WAVE.
