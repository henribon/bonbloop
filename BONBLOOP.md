# BONBLOOP

**BONBLOOP** é um firmware para o **M-VAVE FM-1**, feito a partir do [SLOOP 2.8.5](https://github.com/isod89/sloop-fm1)
(que por sua vez é baseado no [Felucca](https://github.com/hugelton/Felucca)). Ele **parte do SLOOP** — os 4 tracks,
o sequenciador, a bateria, os projetos, o MIDI, o áudio USB e o editor web — e constrói o seu próprio instrumento por
cima, com a sua própria cara:

- **MODO SYNTH**: um sintetizador de **3 osciladores** (wavetables e formas analógicas, com **reverse**), com envelope
  e LFO por oscilador, filtro e uma cadeia de efeitos que você monta (drive estilo Decapitator, phaser, filtro, EQ,
  chorus, delay, reverb).
- **Efeitos em paralelo**: o MIXER de cada track (DST, MOD, DLY, REV ao lado do som seco), um **EQ** e um **soft
  clipper** em cada track, um **COMP** no master; OCT- + OCT+ volta um efeito ao padrão.
- **Bateria**: o drive do synth, o clipper e o **REPEAT** que trava ligado.
- **A escala nas teclas** (GLOBAL → SCALE), só de luz.
- **A cara própria**: a fonte pixel Pixelify Sans, Very Peri sobre preto, violeta para o que está ligado.

Licença: GPL-3.0-only, como o SLOOP e o Felucca (ver `LICENSE` e `LICENSING.md`).

**Versão 1.0** — o **primeiro lançamento oficial**. Junta tudo o que veio nas versões de desenvolvimento abaixo e traz:
**o nível de cada efeito no KNOB 1** de todas as páginas de efeito do modo synth — **MIX** (seco / molhado) no DRIVE,
no PHASER e no DJ FILT, **SEND** no CHORUS, no DELAY e no REVERB —, o **mesmo valor** do fader daquele efeito no **FX
MIX** (ver *O nível de cada efeito: MIX e SEND*). O DST do DRIVE passou a se chamar **DRIVE** e o PUNSH foi para a
página **DRIVE 2**; o EQ não tem mais MIX, e os MIX de retorno do chorus, do delay e do reverb saíram. Um **LOAD** ou
um **NEW** fica guardado na hora: desligou logo depois, o FM-1 volta com ele (antes podia voltar o projeto de antes).
E o estilo do **barquinho** (o logo do SLOOP) saiu do visualizador: agora são 11.

*Antes da 1.0, o BONBLOOP passou por versões de desenvolvimento (1.1 a 1.9.2), não lançadas oficialmente; as notas de
cada uma ficam aqui como histórico.*

**Versão 1.9.2** — a página **CLIP** tem a cara de um *soft clipper*: a **curva de transferência** (entrada × saída
em dB, desenhada pelo próprio código do clipper), o **teto** e os medidores **IN / OUT**. **OCT- + OCT+** numa página
de efeito **volta aquele efeito ao padrão** (ver *Voltar um efeito ao padrão*). E o **NEW** (SAVE → TOOLS) agora zera
**todos** os efeitos: o EQ e o CLIP de cada track, o EQ da bateria e a cadeia do synth (antes eles ficavam).
**O fim da nota** deixou de ser preso ao grid: a gravação guarda **onde você soltou a tecla** (em 1/8 de passo, ~16 ms a
120 BPM em 1/16) e o **LEN** da página **SEQ → NOTES** agora é em **ms** (ver *O fim da nota em ms*). E as luzes da
escala (GLOBAL → SCALE) **apagam enquanto uma camada está segurada** (FX, ARP, SEQ...), para as teclas mostrarem só a
camada. E o BONBLOOP tem **uma cara só**: a paleta lilás **PERI**; as outras cores do SLOOP, o MENU → COLOR e o tema do
editor web saíram.

**Versão 1.9.1** — a escala nas teclas ganhou sua **página no GLOBAL** (**GLOBAL → SCALE**: ROOT, SCALE, LIGHT
até o brilho máximo, MARK), só de luz: não mexe mais no ROOT/SCL da SEL, que podia travar as teclas pretas (ver *As
teclas mostram a escala*). E o **REPEAT** da bateria ganhou uma **trava**: segure o **ARP** sozinho por um instante e
ele fica ligado com a camada do ARP aberta, para mudar a divisão no **KNOB 1** enquanto toca (ver *REPEAT da bateria*).

**Versão 1.9** — **as teclas mostram a escala**: SEL → página SEL 2 → **KEYS** em **SCALE** e as teclas do FM-1 ficam
acesas nas notas do tom e da escala da track selecionada, a tônica mais forte (ver *As teclas mostram a escala*).
E o **EQ** (FX → EQ 1 / EQ 2) passa a ser **de cada track**: mexe só no canal selecionado (na 1.7 e na 1.8 era o EQ
do master e mudava todos os instrumentos). E o **CLIP** (o *soft clipper*) agora existe em **todas as tracks**, não só na
bateria: FX → CLIP com uma track de instrumento selecionada regula o clipper dela.

**Versão 1.8** — uma página **CLIP** para o clipper da bateria (FX → CLIP, com a bateria selecionada): **DRIVE**,
**CEIL** (o teto, 0 a −12 dB), **SHAPE** (SOFT, HARD, FOLD) e **OUT** (−12 a +6 dB). Na paleta PERI, o que está
**ligado ou selecionado** fica **violeta** `#8B5CF6`: MUTE, SOLO, FILL e BAR na camada MIX (GLO segurado), e a track
selecionada na tela TRACKS. **Knobs só para som**: ficam só nas páginas em que todo valor é um parâmetro de som
contínuo (um nível, um tempo, um corte, uma profundidade: LFO DEST, os parâmetros dos motores...). As páginas de
**efeito** e as de **escolher ou ajustar** algo (FX CHAIN, GLOBAL, SYSTEM, MIDI, MASTER, TEXTURE, ARP, PATTERN, um
TYPE, um WAVE...) voltam à tela **dividida em quatro** (o nome pequeno e o valor grande), como antes dos knobs.

**Versão 1.7** — **páginas FX novas** fora do modo synth: o **MIXER** da track (DST · MOD · DLY · REV, tudo em
paralelo), um **COMP** (glue) e um **EQ** de 4 bandas no master, os ajustes de DIST, MOD (chorus / flanger / phaser),
DELAY e REVERB, o FILTER e, por último, o SLICER. No modo synth: **FX MIX** (um knob por efeito da cadeia, no lugar
de TRACK FX + FX MIX), **OSC MIX** (o volume de cada oscilador) e **SAVE** abre a página **PRESET** para salvar o
som; OSC MIX e FX MIX desenhados como **faders** de mixer. O **COMP** tem a tela de um compressor de DAW (RATIO,
ATTACK, RELEASE à esquerda; à direita o medidor com o THRESH e a redução de ganho). A **bateria** ganhou o DRIVE do
synth (estilo Decapitator) no DST e um **soft clipper** (CLIP) no lugar do DLY. O CLICK ON agora se chama **PL+REC**.
Correção: girar um GAIN do EQ ou um MIX passando por 1–6 ligava e desligava efeitos (desde a 1.5).
E uma **cara nova**: a fonte pixel **Pixelify Sans** e a paleta **PERI** (Pantone 17-3938 Very Peri sobre
preto), com **knobs** em toda página que só mostrava números (ver *A cara nova*).

**1.6** — a **FX CHAIN é do projeto** (uma só, para os sons do synth), com os **MIX** de cada efeito; **MIX** também
no reverb e no chorus; **copiar e colar** o padrão de uma track em outra.

**1.5** — um **EQ de 4 bandas** com a curva na tela, como o de uma DAW (FX CHAIN → EQ); o **REC grava em tempo
real** de fábrica (o QUANT da tela REC READY liga a quantização).

**1.4.2** — tercinas do REPEAT gravadas no tempo exato, na grade de 1/16 (o micro-timing de cada passo),
sem mudar a grade do padrão.

**1.4.1** — tercinas do REPEAT gravadas no lugar; REPEAT com 1/4T e 1/2T; o **TRANSIENT saiu** (sobrava pouco
uso para a flash que ocupava: 424 bytes). Os presets WT PLUCK, WT BELLS e DIGI BASS, que o usavam, foram reajustados. E as caudas
já quase mudas de dois samples foram encurtadas (BASS: a partir de 0,8 s, abaixo de -27 dB; VIBES F3: a partir de
0,7 s, abaixo de -40 dB): 7 KB de flash livres para o que vem (o EQ do synth).

**1.4** — **REPEAT** na bateria (toque no ARP); o ARP com **1/2, 1/2T, 1/4T e 1/32T**; o delay com
**todas as tercinas** e um **MODE SYNC / MS**; as divisões em ordem musical no knob; o modo synth **começa num seno puro**.

**1.3.1** — correção: nas páginas OSC2 ENV e OSC3 ENV o desenho do envelope agora acompanha os knobs.

**1.3** — no MODO SYNTH não existe mais o envelope da voz: **cada oscilador tem o seu envelope completo**
(ATK · DEC · SUS · REL, igual ao do SLOOP).
(1.2: LFO por oscilador, cadeia de efeitos que você monta, DRIVE mais pesado, delay 1/2, 1/2T, livre em ms e FIX,
voicings de acorde no SEL, REC + SAVE para limpar o synth. 1.1: 3 osciladores, 16 formas de onda, REV, efeitos do
synth, o modo segura a tela.)

---

## O MODO SYNTH

### Entrar e sair

| Gesto | O que faz |
| --- | --- |
| **Segure ENV ~0,7 s** | Liga o MODO SYNTH. A tela mostra **SYNTH MODE** e abre a página **OSC 1**. |
| **Segure ENV de novo** | Desliga e volta ao SLOOP normal (página ENV). O som do track continua. |

Ao ligar:

- o **track selecionado** passa a tocar o engine **WAVE** no som **SINE** (1.4): só o OSC 1, numa senoide, OSC 2
  e 3 desligados, filtro aberto, sem efeitos e sem LFO — a menos que já esteja em WAVE: aí o som é mantido;
- se a **bateria** (track 4) estiver selecionada, o modo pega o **track 1**;
- o cabeçalho mostra o selo **SYNTH** na cor do track e a linha sob ele fica colorida.

Um toque rápido em ENV continua abrindo a página ENV, como sempre; só **segurar** liga e desliga o modo.

### O modo segura a tela (1.1)

Dentro do MODO SYNTH você **não sai sem querer**:

- **ARP, SEQ, SEL e GLO** tocados rapidamente não abrem nada (a tela avisa *HOLD ENV: SYNTH OFF*).
  **Segurados**, as camadas deles funcionam como sempre (passos no SEQ, mute/solo no GLO, seções no SAVE, roll
  no ARP...) e, ao soltar, você volta ao synth.
- **SAVE** tocado rapidamente abre a página **PRESET** do synth (1.7): **SLOT · LOAD · ERASE · SAVE**, os presets de
  usuário (os mesmos de SAVE → USER). Escolha o slot no KNOB 1 e gire o KNOB 4 duas vezes para salvar o som da track
  (osciladores, envelopes, LFOs, EQ, MIX e envios). Ele aparece no navegador de PRESETS.
- **HOME** volta à página OSC 1 (segurar HOME ainda abre o menu).
- **ALGORITHM** (troca de track) só passa pelos tracks de synth 1 a 3.
- Se algo trocar a tela (carregar um projeto, o editor web), ela volta ao synth.
- PLAY, REC, OCT−/OCT+, o teclado e o sequenciador funcionam normalmente.

### Os botões dentro do modo

Quatro botões abrem os grupos de páginas do synth. Tocar de novo no mesmo botão passa para a próxima página do
grupo; o **SELECT** percorre todas as páginas que estão à mostra.

| Botão | Página | KNOB 1 · KNOB 2 · KNOB 3 · KNOB 4 |
| --- | --- | --- |
| **EDIT** | OSC 1 | WAVE · POS · LVL1 · REV |
| | OSC 2 | WAVE · POS · LVL2 · REV |
| | OSC 3 | WAVE · POS · LVL3 · REV |
| | OSC MIX | LVL1 · LVL2 · LVL3 · LEVEL — **o volume de cada oscilador e o da track**, em faders (1.7) |
| | TUNE | SEMI2 · FINE2 · SEMI3 · FINE3 |
| | FILTER | CUT · RES · TYPE (LP/BP/HP) · FLT (quanto o envelope abre o filtro) |
| | VOICE | WARP · VCE (POLY/MONO/LEG/UNI) · DTUNE (abertura do unison) · GLD |
| **ENV** | OSC1 ENV | ATK · DEC · SUS · REL — **o envelope do OSC 1** (1.3) |
| | OSC2 ENV | ATK · DEC · SUS · REL — **o envelope do OSC 2** |
| | OSC3 ENV | ATK · DEC · SUS · REL — **o envelope do OSC 3** |
| | ENV DEST | FLT · PIT · **POS** · SYNC (o envelope do OSC 1 movendo filtro, pitch e POS; SYNC vale para os três) |
| **LFO** | LFO | RATE · WAVE · PHS · FADE |
| | LFO 2 | SYNC · TRIG · POL |
| | LFO DEST | PIT · FLT · **POS** · AMP |
| | OSC1 LFO · OSC2 LFO · OSC3 LFO | RATE · DEPTH · DEST (POS / PITCH / LVL) — **o LFO de cada oscilador** (1.2) |
| **FX** | FX CHAIN | SLOT1 · SLOT2 · SLOT3 · SLOT4 — **a sua cadeia de efeitos** (1.2) |
| | CHAIN 2 | SLOT5 · SLOT6 · SLOT7 |
| | FX MIX | **um fader por efeito da cadeia, na ordem dela** (1.7): DRIVE, PHASER e FILTER o MIX (seco / molhado), CHORUS, DELAY e REVERB o envio. É **o mesmo valor do KNOB 1 da página de cada efeito**: mexeu num, mexeu no outro (1.0). O EQ não tem MIX (soa sempre inteiro) |
| | FX MIX 2 | os efeitos dos slots 5 a 7 (só aparece com 5 efeitos ou mais) |
| | DRIVE | **MIX** (seco / molhado, o do FX MIX) · **DRIVE** (o quanto distorce; 0 desliga; era o DST) · STYLE (A E N T P) · TONE |
| | DRIVE 2 | PUNSH (1.0: saiu da página DRIVE para o MIX caber) |
| | PHASER | MIX · RATE · DEPTH · FDBK |
| | DJ FILT | **MIX** (o do FX MIX) · FILT (filtro DJ do track) |
| | EQ 1 | LOW (Hz) · GAIN · MID1 (Hz) · GAIN — com a curva (1.5) |
| | EQ 2 | MID2 (Hz) · GAIN · HIGH (Hz) · GAIN — com a curva |
| | CHORUS | **SEND** (o envio, o do FX MIX) · CRT · CDP — o TYPE (chorus / flanger / phaser) fica em FX → MOD |
| | DELAY | DLY (envio) · TIME (1/2 … 1/32T, ou os ms no MODE MS) · FDBK · **MODE** (SYNC / MS) |
| | DELAY 2 | COLR |
| | REVERB | **SEND** (o envio, o do FX MIX) · SIZE · DAMP |
| **SAVE** | PRESET | SLOT · LOAD · ERASE · SAVE — **salvar o som** (1.7) |

As páginas de cada efeito **só aparecem quando ele está na cadeia** (ver abaixo).

- **PRESETS** (o encoder), numa página do synth: passa pelos 16 sons WAVE.
- No engine WAVE, os destinos **SHP** do envelope e do LFO aparecem como **POS**: movem a posição de **todos** os
  osciladores ao mesmo tempo (o *wavetable sweep*).
- Fora do modo, as páginas EDIT do SLOOP mostram, num track WAVE, os campos de verdade (OSC 1 e 2; filtro e OSC 3).

### A cara nova (1.7)

- **Fonte**: **Pixelify Sans** (SIL OFL 1.1), uma fonte pixel, na grade dela mesma: 11 px, maiúsculas de 7 px, cada
  pixel aceso ou apagado (nítida, sem borrão), no mesmo espaço da Terminus de antes: nada mudou de lugar. O texto
  grande é ela em dobro (maiúsculas de 14 px), limpo. Os números têm todos a mesma largura (um valor não "dança"
  enquanto o knob gira). Ocupa bem menos flash que a Terminus (1,9 KB contra 5,4 KB). (A primeira versão da 1.7
  usava a Inter Tight suavizada: sem *hinting*, borrava nesse tamanho.)
- **Paleta PERI** (o novo padrão, no lugar da MONO): **Very Peri** `#6667AB` e preto. Rótulos e preenchimentos em
  Very Peri, valores em lilás claro `#D9DAF4`, linhas em roxo bem escuro; o que você gira fica branco. Desde a
  **1.9.2** é a **única**: GREEN, AMBER, CYAN e RED, o **MENU → COLOR** e o tema do editor web saíram (o MENU → SCREEN
  ficou com ZOOM e KNOB ACCEL; um tema mandado antes fica guardado nos backups, mas não aparece).
- **Knob ou fader onde é som** (1.8: só lá): as páginas de parâmetros de som contínuos mostram uma **fileira de
  knobs**, um embaixo de cada coluna, com o valor e o nome; as de escolha, de ajuste e de efeito, a tela dividida em
  quatro de antes; uma página de
  um valor só, **um knob grande**. Os mixers (FX → MIXER, OSC MIX, FX MIX) são **faders**. As telas com desenho
  próprio (envelope, LFO, curva do EQ, wavetable, passos) continuam com ele.
- **Verde puxado pro roxo** `#3BA488` na paleta PERI: o arco aceso dos knobs, a barrinha embaixo das colunas e o
  nível dos faders.
- **Nomes e valores**: os nomes (e as unidades) num espaço menor, os valores em destaque; a cor separa um do outro.
- **COMP**: RATIO, ATTACK e RELEASE como knobs; o medidor L/R **verde** (mais claro acima de −3 dB).
- **Boot**: BON em lilás claro, BLOOP em Very Peri.

### A tela

As páginas OSC 1, 2 e 3 desenham **o próprio oscilador**: numa tabela, os 8 frames um atrás do outro (cinza) e, na
frente, na cor do track, a forma de onda **na posição que está tocando** (com a modulação, enquanto uma nota
soa); numa forma analógica, dois ciclos dela (no SQR, com a largura de pulso do POS).

---

## O engine WAVE

### Os 3 osciladores

Cada oscilador tem:

| Campo | O que faz |
| --- | --- |
| **WAVE** | `OFF`, `SIN`, `TRI`, `SAW`, `SQR`, `NOISE`, ou uma das 10 tabelas (abaixo). |
| **POS** | Numa tabela: o frame (1 a 8, lido entre dois). No `SQR`: a largura do pulso (50 % → 6 %). No `NOISE`: a cor (branco → escuro). |
| **LVL** | O volume dele na mistura. |
| **REV** | Toca o oscilador **ao contrário** (abaixo). |
| **SEMI / FINE** | Só OSC 2 e 3: afinação de −24 a +24 semitons e −16 a +15 cents em relação ao OSC 1. |

`SAW` e `SQR` são formas analógicas sem aliasing (polyBLEP, como as do engine ANALOG): graves cheios. As tabelas
têm até 24 harmônicos — o caráter dos wavetables dos anos 80.

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

**WARP** (página VOICE) dobra a fase de leitura de todas as tabelas: a primeira metade do ciclo é espremida (mais
brilho, um "PWM" em qualquer tabela). O DC que isso criaria é compensado.

### REV — tocar ao contrário

Num sampler, tocar ao contrário transforma um som que decai num som que **cresce e para de repente**. Num
oscilador de um só ciclo, ler a onda de trás para frente quase não muda o timbre — o que o ouvido percebe como
"reverse" é a **evolução da nota ao contrário**. Por isso o REV faz as duas coisas:

1. lê o ciclo do oscilador de trás para frente;
2. inverte o decaimento: o oscilador **começa em −40 dB e cresce** no tempo do **DEC** até o volume cheio, e aí
   **para** (cai para o nível do SUS — com SUS 0, silêncio, como o fim de um sample invertido). Ao soltar a
   tecla, ele some junto com os outros (no tempo do REL).

Com um oscilador normal e outro em REV, você tem o clássico "pluck + reverse". O som **REV SWELL** mostra isso.

### Filtro

**CUT** e **RES** (o filtro SVF do SLOOP), **TYPE** passa-baixa, passa-banda ou passa-alta, com 50 % de key
tracking; o envelope e o LFO (→ FLT) movem o CUT. Na mesma página, o **FILT** é o filtro DJ do track (passa-baixa
para a esquerda, passa-alta para a direita), depois de todos os efeitos de insert. O CC 74 de um teclado MIDI
move o filtro DJ.

### Um envelope para cada oscilador (1.3)

No MODO SYNTH **não existe envelope principal**: o ENV abre **OSC1 ENV**, **OSC2 ENV** e **OSC3 ENV**, cada um com
**ATK · DEC · SUS · REL** e o desenho do envelope, exatamente como o ENV do SLOOP (mesmas faixas, mesmas curvas, e o
**SYNC** da página ENV DEST vale para os três).

- O volume de cada oscilador segue **só o envelope dele**: um pad pode entrar devagar no OSC 2 enquanto o OSC 1 dá
  o ataque, ou o OSC 3 soar só no começo, como um clique.
- A nota dura até o **último** oscilador terminar o release: um REL longo no OSC 2 continua soando depois que o
  OSC 1 já parou.
- Um preset começa com os três envelopes iguais (o do preset); daí cada um vai para o seu lado.
- O envelope do OSC 1 é o ENV do track (P_ATK … P_REL): fora do modo synth, a página ENV do SLOOP num track WAVE
  mexe nele. É ele também que move filtro, pitch e POS (ENV DEST).
- Com **REV**, o envelope daquele oscilador vira o swell ao contrário (sobe na velocidade do DEC dele até o topo e
  cai no SUS dele).

### LFO de cada oscilador (1.2)

Além do LFO da voz (que vale para todos), cada oscilador tem o seu, na página **OSCn LFO**: **RATE** (0–15),
**DEPTH** (−8 a +7; 0 desliga) e **DEST**: **POS** (varre a tabela só daquele oscilador), **PITCH** (vibrato só nele)
ou **LVL** (tremolo só nele).

### A cadeia de efeitos (1.2; do projeto desde a 1.6)

**A FX CHAIN é uma só, do projeto (1.6):** os efeitos escolhidos e a ordem deles valem para **todas as tracks com som
do synth** (1.7: só elas; as outras tracks usam as páginas FX normais, ver *Efeitos fora do modo synth*). Cada track
regula **quanto** de cada efeito na página **FX MIX** (1.7): um knob por efeito, na ordem da cadeia — o **MIX**
seco/molhado de DRIVE, PHASER, FILTER e EQ, o **envio** de CHORUS, DELAY e REVERB. O EQ tem as bandas de cada track.
Tirar um efeito da cadeia desliga ele nas três tracks. Num projeto salvo antes da 1.6, a cadeia que vale é a que
estava na track 1.


No MODO SYNTH **todo efeito começa desligado**. A primeira página do FX é a **FX CHAIN**: 7 slots (4 aqui, 3 em
CHAIN 2, um por efeito), cada um com **--** ou um efeito — DRIVE, PHASR, FILTR, CHORS, DELAY, REVRB, EQ.

- Escolher um efeito num slot **liga** o efeito (numa quantidade que já se ouve) e faz a página de detalhes dele
  aparecer depois de CHAIN 2. Voltar o slot para **--** desliga o efeito e esconde a página.
- **DRIVE, PHASER e FILTER** tocam **na ordem dos slots** (ex.: PHASER antes do DRIVE soa diferente de DRIVE antes
  do PHASER). CHORUS, DELAY e REVERB são envios (depois de todos).
- Um efeito ligado por fora do modo (nas páginas normais do SLOOP, ou num projeto antigo) entra sozinho no primeiro
  slot livre: a cadeia sempre mostra tudo o que está soando.

| Efeito | O que é |
| --- | --- |
| **DRIVE** | Estilo **Decapitator**, num track WAVE: 5 estilos — **A** fita, **E** válvula (harmônicos pares), **N** transformador, **T** triodo, **P** pentodo —, **TONE** (escuro ↔ aberto) e **PUNSH**. No 1.2 ficou bem mais pesado: mais ganho em todo o curso do DST e o PUNSH soma +18 dB. |
| **PHASER** | O INSERT do SLOOP como PHASR (MIX · RATE · DEPTH · FDBK). |
| **FILTER** | O filtro DJ do track (passa-baixa para a esquerda, passa-alta para a direita). |
| **CHORUS, DELAY, REVERB** | Os envios do SLOOP e os ajustes globais deles, na mesma página. |

### EQ de 4 bandas (1.5)

Ponha **EQ** num slot da FX CHAIN e aparecem as páginas **EQ 1** e **EQ 2**, com a **curva** do EQ desenhada como numa
DAW: o ganho (±15 dB) em função da frequência (30 Hz a 16 kHz), com linhas em 0 dB e em 100 Hz, 1 kHz e 10 kHz.

| Banda | Tipo | Knobs |
| --- | --- | --- |
| **LOW** | shelf (graves) | EQ 1: KNOB 1 frequência, KNOB 2 ganho |
| **MID1** | sino | EQ 1: KNOB 3 frequência, KNOB 4 ganho |
| **MID2** | sino | EQ 2: KNOB 1 frequência, KNOB 2 ganho |
| **HIGH** | shelf (agudos) | EQ 2: KNOB 3 frequência, KNOB 4 ganho |

- Cada banda é um **ponto** no gráfico, na frequência e no ganho dela, na cor do seu knob de frequência (azul KNOB 1,
  amarelo KNOB 3); as da outra página aparecem em cinza.
- O EQ começa **plano** (100 Hz, 500 Hz, 2 kHz, 8 kHz, todos em 0 dB) e fica na ordem da cadeia junto com DRIVE,
  PHASER e FILTER. Tirar o EQ da cadeia desliga, sem perder os ajustes.
- Os filtros são os SVF do Andrew Simper (os mesmos do filtro do synth): shelves sem o "afundamento" depois do corte.
  Uma banda em 0 dB não gasta nada.
- O EQ fica salvo com o som: nos projetos e nos presets de usuário.

### Delay: MODE SYNC / MS (1.4)

O delay tem um **MODE** (KNOB 4 da página DELAY, dentro e fora do modo synth):

- **SYNC**: o **TIME** é uma figura, no BPM: **1/2, 1/2T, 1/4, 1/4T, 1/4D, 1/8, 1/8T, 1/8D, 1/16, 1/16T, 1/16D,
  1/32, 1/32T** — o knob anda nessa ordem, uma figura por clique.
- **MS**: o **TIME** vira **ms** (10–1480) e não muda com o BPM. Ao passar para MS, o tempo que estava tocando vira
  ms (é o antigo FIX: mude o andamento e o eco fica onde estava). De volta a SYNC, volta a figura de antes.

### Limpar o synth: REC + SAVE (1.2)

No MODO SYNTH, segure **SAVE** e **REC** juntos: depois de ~0,7 s aparece o anel **clear / synth sound**; segure
até ele fechar (~1,3 s) e o track volta ao **SINE**, com todos os efeitos desligados, a cadeia
vazia, sem LFO nos osciladores e os três envelopes iguais ao do som. **As notas do track ficam** (para apagar as notas, REC sozinho segurado,
como no SLOOP). Soltou antes: nada acontece.

### Sem gastar flash nem RAM

O flash do FM-1 está quase cheio, então as formas de onda **não ficam no flash**: cada tabela escolhida é
**calculada** dentro da "arena" que os engines GRAIN/FM6/PHYS já compartilham por track — uma tabela para cada
oscilador, 8 frames de 128 amostras —, aos poucos no áudio (~23 ms por tabela), substituindo frame a frame a
que está tocando; trocar de tabela nunca deixa o som mudo. Nenhum byte de RAM a mais (um `_Static_assert`
garante isso).

Os 25 parâmetros do synth cabem **empacotados nos 8 parâmetros do engine** (16 bits cada): projetos, presets de
usuário e p-locks os guardam como sempre. Os do 1.2 (envelope e LFO de cada oscilador, a cadeia: 12 bytes por track,
e o MS do delay) vão num registro ao lado de cada projeto e de cada banco de presets de usuário na flash, que o
SLOOP nunca lê. As páginas do synth editam cada campo; o **editor web** mostra os 8
números brutos e não edita esses campos.

Custo de CPU, medido no PC: 1 oscilador de tabela ≈ 0,9× uma voz do ANALOG; 3 osciladores de tabela ≈ 1,5×.

### Os 16 sons

WT BASIC, WT SWEEP, PPG PAD, VOX SWEEP, WT CHOIR, PWM STRNG, WT ORGAN, WT PLUCK, WT BELLS, RESO BASS,
DIGI BASS, FM GROWL, FOLD LEAD, SUPER WAVE, CHIP WAVE e **REV SWELL** — no navegador de PRESETS por categoria
(BASS, ORGN, PAD, LEAD, PLCK), com volume equalizado aos do ANALOG (`WAVE_TRIM`, a ±2 dB).

---

## Fora do modo synth

### Efeitos fora do modo synth (1.7)

O botão **FX** abre, em ordem (toque de novo no FX ou use o SELECT para passar):

| Página | KNOB 1 · KNOB 2 · KNOB 3 · KNOB 4 | De quem |
| --- | --- | --- |
| **MIXER** | DST · MOD · DLY · REV, em faders (na bateria: DST · MOD · **CLIP** · REV) | da track |
| **COMP** | THRS (0 dB = desligado, até −40 dB) · RATIO (2:1, 4:1, 10:1) · ATK (0,1–30 ms) · REL (0,1–1,2 s) | do master |
| **EQ 1** / **EQ 2** | LOW · GAIN · MID1 · GAIN / MID2 · GAIN · HIGH · GAIN, com a curva | da track (1.9) |
| **DIST** | TYPE (SOFT, HARD, FOLD, FUZZ, CRUSH) · DRIVE · TONE · LEVEL (no CRUSH: BITS · RATE · LPF); na bateria: STYLE · TONE · PUNSH | do projeto |
| **CLIP** (1.8 na bateria; 1.9 em todas) | DRIVE · CEIL (0 a −12 dB) · SHAPE (SOFT, HARD, FOLD) · OUT (−12 a +6 dB) | da track |
| **MOD** | TYPE (CHOR, FLANG, PHASR) · RATE · DEPTH · FDBK | do projeto |
| **DELAY** | TIME · FDBK · COLR · MODE (SYNC / MS) | do projeto |
| **REVERB** | SIZE · DAMP | do projeto |
| **FILTER** | FILT (o filtro DJ da track) | da track |
| **SLICER** | SLCR · PAT · RATE · DEPTH | da track |

- **Tudo em paralelo:** no MIXER, cada knob soma o efeito ao som seco, que fica inteiro. **DST** soma o som
  distorcido (o DIST da página DIST); **MOD**, **DLY** e **REV** são os envios para o MOD, o delay e o reverb. Em 0, a
  track sai exatamente como sem o efeito.
- O **MOD** é o barramento do chorus, agora com três tipos: **CHOR** (o chorus de sempre), **FLANG** e **PHASR**
  (estéreo, um lado meia volta atrás do outro). O FDBK vale para FLANG e PHASR.
- O **COMP** é um *glue compressor* no master (depois dos efeitos, antes do volume): estéreo, com make-up automático
  (devolve metade do que o compressor tira em fundo de escala). Em THRS 0 dB não faz nada. A tela é a de um compressor
  de DAW: RATIO, ATTACK e RELEASE à esquerda; à direita o medidor de 0 a −48 dB com a entrada (esquerda e direita:
  verde, mais claro acima de −3 dB), a **redução de ganho** descendo do topo e o **THRESH** como uma linha com uma
  seta e o valor ao lado.
- O **EQ** (1.9) é **de cada track**, inclusive da bateria: o mesmo EQ de 4 bandas do modo synth (numa track do synth
  é o mesmo EQ da cadeia; fora da cadeia ele toca depois dela). Plano, não gasta nada. (Na 1.7 e na 1.8 ele era do
  master e mudava a mistura inteira; num projeto dessas versões, aquele EQ vira o da bateria.)
- Na **bateria**: o **DST** é o DRIVE do modo synth (estilo Decapitator), em paralelo, com STYLE (A E N T P), TONE e
  PUNSH na página **DIST** (com a bateria selecionada); **MOD** é dela; no lugar do DLY fica o **CLIP**, um *soft clipper*
  (empurra a bateria num tanh e devolve metade do ganho: os golpes baixos um pouco mais fortes, os picos arredondados);
  **REV** é o envio da bateria (o mesmo de GLO → DRUMS). O envio de delay da bateria continua em GLO → DRUMS.
- **CLIP** (1.8; 1.9 em todas as tracks), o clipper da track selecionada: na **bateria**, **DRIVE** é o mesmo
  CLIP do MIXER (0 desliga o clipper); numa **track de instrumento** é o clipper dela, no fim da cadeia de efeitos
  dela, antes do volume (fica com a track quando você troca de som; não vai nos presets de usuário); **CEIL** baixa o teto onde ele arredonda (0 a −12 dB); **SHAPE** escolhe o
  corte: **SOFT** (o tanh de sempre), **HARD** (corta reto no teto) ou **FOLD** (dobra de volta no teto: mais
  harmônicos, mais sujo); **OUT** ajusta o volume na saída (−12 a +6 dB), para comparar com e sem. Em CEIL 0 dB, SOFT e
  OUT 0 dB ele soa exatamente como o CLIP da 1.7. Fica salvo no projeto; um projeto da 1.7 abre com esses três nos
  padrões.
- **A tela do CLIP (1.9.2)**: à esquerda, a **curva de transferência** em dB (entrada de −30 a +6 dBFS na
  horizontal, saída na vertical, uma linha a cada 6 dB, a de 0 dB mais clara): sobe a 45° com o ganho do DRIVE e
  dobra no **teto** (a linha violeta, CEIL) — arredondada no SOFT, em quina no HARD, voltando para baixo no FOLD. À
  direita, os medidores **IN** e **OUT** da track selecionada (enquanto o clipper trabalha).
- **Sem MIX nestas páginas:** os MIX (seco/molhado e retornos) ficam só no modo synth.

### Voltar um efeito ao padrão (1.9.2)

Numa página de efeito (FX → MIXER, COMP, EQ, DIST, CLIP, MOD, DELAY, REVERB, FILTER, SLICER, e as páginas de efeito do
modo synth), aperte **OCT- e OCT+ juntos**: aquele efeito volta ao padrão (da track selecionada, ou do projeto, como a
página mostra) e a tela diz **RESET**. No EQ, as duas páginas voltam juntas: plano, com as bandas onde começam. No
DELAY, o MODE volta a SYNC. A oitava fica como estava. Fora das páginas de efeito, OCT- + OCT+ continua voltando a
oitava para 0.
- O antigo **INSERT** saiu destas páginas: as distorções dele são o DIST, e PHASR / FLANG / CHOR são o MOD. Ele continua
  existindo só como o PHASER da cadeia do synth. Num som antigo com INSERT, ele deixa de soar nas tracks do SLOOP; o
  DST antigo agora soa em paralelo, com o DIST do projeto.
- Tudo fica salvo no projeto (num projeto antigo: COMP desligado, MOD em CHOR, EQ do master plano).

### CLICK: PL+REC (1.7)

O metrônomo fica em **GLO → GLOBAL → KNOB 3 (CLICK)**: **OFF**, **REC** (só gravando) ou **PL+REC** (tocando e
gravando; antes se chamava ON).

### Copiar e colar um padrão (1.6)

**SAVE → TOOLS → COPY** (KNOB 3) guarda o padrão da track selecionada: os passos, o micro-timing, as condições de
FILL, os locks e LEN, DIV, SWING e GATE. Selecione outra track e **SAVE → TOOLS 2 → PASTE** (KNOB 1; gire duas vezes
para confirmar) cola nela. De uma track de synth para outra de synth, da bateria para a bateria (de uma para a outra a
tela avisa *DRUMS / SYNTH: NO*). Os locks só vão junto se a outra track tiver o mesmo engine. **EDIT + OCT−** desfaz o
PASTE.

### O nível de cada efeito: MIX e SEND (1.0)

No modo synth, **o KNOB 1 de toda página de efeito é o nível dele**, e é **o mesmo valor** do fader daquele efeito no
**FX MIX** (mexeu num, o outro acompanha):

- **DRIVE, PHASER e DJ FILT** são efeitos de inserção: o KNOB 1 é o **MIX** (seco / molhado). No DRIVE, o KNOB 2
  **DRIVE** é *quanto* distorce; o MIX é *quanto da distorção* aparece.
- **CHORUS, DELAY e REVERB** são envios: o KNOB 1 é o **SEND**, quanto deste track vai para o efeito. O retorno deles
  ficou fixo (o chorus e o reverb inteiros, o delay no nível de sempre do SLOOP): só há um nível para mexer.
- O **EQ** não tem MIX: soa sempre inteiro (um EQ meio seco quase nunca é útil).

Da 1.6 à 1.9.2 o chorus, o delay e o reverb tinham também um MIX de retorno nas páginas deles, e o DRIVE chamava a
distorção de DST: dois "níveis" com nomes parecidos para coisas diferentes.

### REC em tempo real (1.5)

O **REC grava em tempo real** de fábrica: cada nota fica no passo mais perto e o **micro-timing** do passo (1/64 de um
passo, ~2 ms a 120 BPM em 1/16) guarda o tempo em que você tocou. Para gravar **quantizado**, na tela **REC READY**
(REC com o transporte parado) gire o **KNOB 4 (QUANT)** para a direita: **ON**; para a esquerda volta a **OFF**. A
escolha fica salva no FM-1. Um passo tem um micro-timing só: num acorde, ou com dois sons da bateria no mesmo passo,
vale o da primeira nota.

### O fim da nota em ms (1.9.2)

Até a 1.9.1 uma nota acabava no **GATE** do padrão: gravada, ela ocupava passos inteiros e, se você soltava a tecla
antes do meio do passo seguinte, ela perdia esse passo (uma nota de 1,4 passo tocava meio passo). Agora cada nota tem
o **seu próprio fim**:

- **Gravando** (ao vivo ou no FREE TAKE), o FM-1 guarda onde você **soltou a tecla**, em **1/8 de passo** (~16 ms a
  120 BPM em 1/16), contado de onde a nota toca (com o micro-timing dela). A nota segue nos passos em que você a
  segurou (TIEs) e acaba no ponto exato do último; arredonda para cima, então nunca fica mais curta do que você tocou.
  Só passando o começo de um passo por um fio (até 1/16 dele) não cria um TIE à toa.
- Num **acorde** soltado de uma vez, todas acabam juntas nesse ponto. Uma nota solta enquanto outras seguem presas
  (POLY) acaba no passo mais perto, como antes (um passo tem um fim só).
- **SEQ → NOTES**, **KNOB 3 (LEN)** mostra a duração da nota em **ms** e cada clique muda o fim em **1/8 de passo**;
  passando do fim do passo, a nota ganha o próximo (um TIE), e voltando ela o perde. Girando rápido, anda até 2 passos
  por clique. Uma nota com ratchet anda de passo em passo (cada batida usa o GATE).
- Sem fim próprio (notas de antes, ou programadas nos passos) vale o **GATE**, como sempre. O desenho das notas na
  página NOTES mostra o fim exato.
- O fim fica no **projeto** (no registro extra, depois do que a 1.9.1 lê): um projeto da 1.9.2 aberto na 1.9.1 ou no
  SLOOP toca as notas no GATE.

### REPEAT da bateria (1.4)

Com a **bateria** (track 4) selecionada, um **toque no ARP** liga o **REPEAT** (o ARP fica aceso): cada tecla
segurada repete no tempo, como o *note repeat* de uma drum machine, sem precisar segurar nada. Com **REC** ligado,
as batidas são gravadas (mais rápidas que o passo viram ratchets). Outro toque no ARP desliga.

**Trava do REPEAT (1.9.1).** Com a bateria selecionada, **segure o ARP sozinho** (sem tecla) por uns 0,7 s: o REPEAT
liga e a **camada do ARP fica travada aberta** (o LED do ARP pisca). Solte o ARP: as teclas fazem o repeat e o
**KNOB 1** muda a divisão enquanto você toca, com as duas mãos livres. **Toque no ARP de novo** para destravar e
desligar o REPEAT (outro botão, exceto PLAY, REC e OCT, só destrava).

A divisão é a do roll: **ARP + KNOB 1** — **1/2T, 1/4, 1/4T, 1/8, 1/8T, 1/16, 1/16T, 1/32, 1/32T, 1/64** (1.4.1: 1/4T
e 1/2T). Segurar ARP + teclas continua sendo o roll de sempre, em qualquer track.

**Tercinas gravadas (1.4.2).** O REPEAT toca cada figura no tempo exato (a 480 ticks por semínima: 1/2T = 640,
1/4T = 320, 1/8T = 160, 1/16T = 80). Ao gravar, cada batida vai para o passo mais perto e o **micro-timing (nudge)**
desse passo guarda o tempo dela: **1/2T, 1/4T e 1/8T ficam exatas na grade normal de 1/16**. Um passo tem um nudge só:
se ele já tem outra batida, a nova fica na grade e a tela avisa **TRIPLETS: DIV 16T**; o 1/16T põe duas batidas num
passo de 1/16, então para ele use PATTERN → DIV 16T. 1/32T grava certo em 1/16 (três por passo, ratchet x3). A grade
do padrão nunca muda sozinha (a 1.4.1 mudava para 16T, e aí até o 1/16 tocava em tercina).

### ARP: todas as tercinas (1.4)

O **RATE** do arpejador ganhou **1/2, 1/2T, 1/4T e 1/32T** e anda em ordem musical: 1/2, 1/2T, 1/4, 1/4T, 1/8, 1/8T,
1/16, 1/16T, 1/32, 1/32T. Os valores antigos continuam com o mesmo número (projetos abrem iguais).

### VOICING (1.2)

Com o **CHORD** ligado (SEL → página SEL 2), o antigo **VLEAD** virou **VOIC**, o voicing do acorde:

| VOIC | Um C maior (C E G) vira |
| --- | --- |
| OFF | como tocado |
| LEAD | cada acorde na inversão mais perto do anterior (o VLEAD ON de antes) |
| INV1 / INV2 | 1ª / 2ª inversão (E G C / G C E) |
| DROP2 / DROP3 | a 2ª / 3ª voz de cima uma oitava abaixo (E3 C G / C3 E G) |
| SPRD | aberto: vozes alternadas uma oitava acima (C G E5) |
| +BASS | mais a fundamental uma oitava abaixo (C3 C E G) |

Funciona nas teclas, nos passos e com os modificadores das teclas pretas.

### As teclas mostram a escala (GLOBAL → SCALE, 1.9.1)

A página **SCALE** do GLOBAL (GLO, depois SELECT: logo depois de GLOBAL) mostra uma escala nas teclas do FM-1. É **só
luz**: não muda o que as teclas tocam (o ROOT, SCL e QNT da página SEL ficam como estão; na 1.9 isso ficava na SEL e
mexer na escala de lá podia travar as teclas pretas).

| Knob | O quê |
| --- | --- |
| **ROOT** | a nota da escala, C a B |
| **SCALE** | o tipo: MAJ, MIN, DOR, MIX, PEN, MPEN, HARM, PHRY, LYD, LOC, MEL, BLUES, WHOLE, DIMHW, DIMWH (CHR: todas) |
| **LIGHT** | **OFF**, **LOW** (luz de fundo), **MID** (luz média), **MAX** (a luz mais forte, a mesma de uma tecla tocada) |
| **MARK** | o que fica mais forte: **ROOT** (a tônica), **1-3-5** (1ª, 3ª e 5ª notas da escala) ou **OFF** |

- As notas destacadas pelo MARK ficam um nível acima das outras; no **MAX** (que já é o máximo) elas **piscam**.
- No **MAX**, uma tecla da escala que você toca **apaga** enquanto está pressionada, e uma tecla fora da escala
  **acende**: dá para ver o que você está tocando.
- As luzes seguem o que **cada tecla toca** na track selecionada (com o **TRN**, as teclas acesas andam junto).
- Na **bateria** nada acende (lá as teclas são sons).
- Funciona com MENU → LIGHTS desligado. É um ajuste do FM-1, não do projeto: fica como está ao abrir outro projeto.
- MENU → LIGHTS → KEYS voltou a ter só OFF, C KEYS, WHITE KEYS e ALL KEYS.

--- | --- |
| OFF | nenhuma |
| C | os Dós |
| WHITE | as brancas |
| ALL | todas |
| **SCALE** | **as notas do tom e da escala da track selecionada** |

- Em **SCALE**, as notas da escala ficam com luz fraca e a **tônica** (o ROOT) com luz média, para você se localizar;
  as notas que você toca continuam acendendo forte por cima.
- O tom e a escala são os da página **SEL** (ROOT, SCL) da track selecionada — escolha com SEL + uma tecla, que põe
  todas as tracks no mesmo tom. Trocou de track, as luzes acompanham.
- As luzes mostram o que **cada tecla toca agora**: com **TRN** (transposição), as teclas acesas andam junto; com
  **QNT** em SNAP ou WHITE, acendem as teclas que caem na escala.
- Na **bateria** nada acende (lá as teclas são sons).
- Precisa de LIGHTS ligado (MENU → LIGHTS): ao escolher uma opção de KEYS com ele em OFF, ele vai para LOW.
- É um ajuste do FM-1, não do projeto: fica como está ao abrir outro projeto.

---

## Compatibilidade

- Projetos, presets de usuário, samples e ajustes do SLOOP continuam valendo.
- Sons WAVE salvos com o BONBLOOP 1.0 tocam diferente no 1.1 (os parâmetros do engine mudaram de significado).
- Do 1.1 para o 1.2: os sons e projetos abrem iguais; os efeitos que estavam ligados aparecem sozinhos na cadeia.
  Os presets de fábrica WAVE agora começam sem efeitos (no 1.1 vinham com o phaser no INSERT).
- Do 1.2 para o 1.3: o envelope ON/OFF de 4 bits por oscilador virou um envelope completo; os extras salvos no 1.2
  (LFO dos osciladores, cadeia, MS) não são lidos: o som abre com os três envelopes iguais e os efeitos ligados na cadeia.
- Um projeto salvo no 1.3 aberto no SLOOP ou num BONBLOOP anterior perde só os extras (envelopes do OSC 2 e 3, LFO
  dos osciladores, cadeia, MS): o OSC 1 segue o ENV do track.
- Um projeto com um track em WAVE (engine 12), aberto num SLOOP oficial, cai no som padrão daquele track.
- Um projeto da 1.9.2 aberto na 1.9.1 ou no SLOOP abre igual, só as notas com fim próprio (*O fim da nota em ms*)
  acabam no GATE do padrão.
- Na 1.0 os retornos do chorus e do reverb ficam inteiros e o do delay no nível padrão: um projeto em que esses MIX
  estavam mais baixos toca esses efeitos mais alto (com os valores padrão, nada muda).
- O editor web lê a lista de engines do próprio aparelho, então o WAVE aparece nele (com os números brutos).
- A identidade do pacote é **`FM-1_962`** (1.7), só do BONBLOOP, em todas as versões (até a 1.7 era `FM-1_900`, a do
  SLOOP, que vários firmwares dividem). Instala por cima do SLOOP e de qualquer versão anterior, pelo mesmo instalador.

## Instalar

Pelo navegador (Chrome ou Edge), com o FM-1 ligado no computador pelo USB:
**[bonbap.com.br/bonbloop](https://bonbap.com.br/bonbloop/)** → **INSTALL**. O site (pasta `docs/`, em
inglês) é gerado por `web/make_site.py` e instala direto no FM-1 conectado, pelo Web MIDI.

Imagens para divulgação: `assets/promo/` (`tools/gen_bonbloop_brand.py promo`, com a fonte e as telas do próprio FM-1).

## Compilar e instalar pela linha de comando

Igual ao SLOOP (ver [BUILDING.md](BUILDING.md)): toolchain JieLi (Linux x86-64, via WSL ou Docker) e três
arquivos do SDK AC79. No Windows com WSL:

```
python tools/build_windows.py --distro Ubuntu --toolchain /root/.jieli/toolchain --sdk build/deps/ac79 --slice 0
python tools/fm1_install.py build/felucca.fwsc
```

Tamanho do 1.0: 577.512 de 581.564 bytes de flash (4,0 KB livres; para caber, o desenho de texto ficou só com o
caminho de 1 bit, a Pixelify Sans, 1 bit por pixel, ocupa bem menos que a Terminus; o estado da bateria começa
zerado, sem ocupar flash com o valor inicial; uma paleta só; e sem o estilo do barquinho); RAM 60.924 de 98.304
bytes, pool igual ao do SLOOP.
Para caber, a 1.7 encurtou mais duas caudas já baixas do VIBES (D4 a partir de 0,9 s, abaixo de −37 dB; B4 a partir de
0,93 s, abaixo de −27 dB).

## Testes

`tests/run_tests.sh` roda tudo, incluindo os dois testes novos:

- `tests/wave_test.c` — o engine sozinho: os 24 campos empacotados, todas as tabelas em cada oscilador, todos os
  presets (limites, DC, volume), as formas varridas, REV, DRIVE, extremos, custo de CPU; grava
  `wave_presets.wav` e `wave_sweep.wav` para ouvir.
- `tests/note_end_test.c` (1.9.2) — o fim da nota: segurada 3/8, 6/8, 9/8, 12/8 e 20/8 de passo gravando, ela toca
  essa duração (até 1/8 a mais); o mesmo no FREE TAKE (POLY e LEGATO); o ratchet com o GATE; salvar e abrir, e um
  projeto da 1.9.1 (sem fins).
- `tests/synth_mode_test.c` — a interface real: segurar ENV, os 4 grupos e as 18 páginas, cada knob mexendo
  um campo só, os 3 osciladores tocando, os efeitos, o modo segurando a tela (ARP/SEQ/SEL/GLO/SAVE/HOME/ALGO),
  sair, a bateria selecionada; grava as telas em `synth-*.ppm`.

A suíte de regressão (`tests/regress.c`) confere as 199 renderizações bit a bit com o `golden.txt`. Na 1.7 mudaram
só as que usam o DST (os presets de fábrica com distorção, os testes de envio: o DST agora é em paralelo) e as duas
que tocam o VIBES (as caudas mais curtas).

## Créditos

- **SLOOP** — isod89 e colaboradores (GPL-3.0).
- **Felucca** — Leo Kuroshita / Hügelton Instruments (GPL-3.0).
- **Decapitator** é da Soundtoys; o DRIVE do BONBLOOP só se inspira nos estilos dele.
- **BONBLOOP** — o MODO SYNTH e o engine WAVE.
