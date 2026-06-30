#set page(
  width: 21cm,
  height: auto,
  margin: (x: 2cm, y: 2.5cm),
)

#set par(
  justify: true,
  leading: 0.52em,
)

#set text(lang: "pt", region: "br")

#v(1em)
#align(center, text(size: 2em, weight: "bold")[Relatório de Desenvolvimento - 'Fase 5'])
#v(1em)

#text(
  size: 1.2em,
)[Desenvolvimento de Software #h(1fr) --- #h(1fr) Prof. Dr. Marcelo Soares Pimenta #h(1fr) --- #h(1fr) #text(size: .91em)[INF]01120] \
#text(size: 1.2em)[#underline[Davi Santos] #h(1fr) #underline[00599530]] \
#text(size: 1.2em)[#underline[Gabriel Schons] #h(1fr) #underline[00342020]] \
#text(size: 1.2em)[#underline[Gabriel Copatti] #h(1fr) #underline[00342808]] \
#text(size: 1.2em)[#underline[Julio Augusto de Castilhos Borges] #h(1fr) #underline[00590537]]

#v(1em)
*Link para o repositório do trabalho:* #link("https://github.com/Davi-FSantos/INF-01120_26-1")
#v(1em)

= Introdução e Suposições de Projeto

Este relatório documenta a entrega final da *Music Machine*, englobando todo o histórico de requisitos e classes das Fases 1 e 2, bem como as ações corretivas de refatoração tomadas a partir da revisão de código realizada pelo Grupo Revisor 4.

Para a realização do trabalho, foram estabelecidas as seguintes suposições:
1. *Bancos de Timbres (SoundFonts)*: O motor de síntese FluidSynth exige um arquivo de SoundFont (`.sf2` ou `.sf3`) General MIDI. O sistema procura automaticamente em diretórios padrões de áudio do Linux. Caso não encontre, provê uma caixa de diálogo interativa (`QFileDialog`) para indicação manual.
2. *Reserva do Canal MIDI 9*: O canal MIDI de índice 9 é reservado por especificação para percussão. Para evitar conflitos, o mapeamento de canais pula o índice 9 e cicla entre os demais.
3. *Limites de BPM*: O BPM é controlado globalmente, limitado no intervalo de 40 a 1200 batidas por minuto.

= Lista de Requisitos do Sistema

== Requisitos Funcionais - Fase 1 (Original)
1. Fornecer um campo de entrada de texto para inserir ou colar sequências.
2. Mapear caracteres específicos para instruções musicais definidas.
3. Permitir configurar parâmetros iniciais (BPM, instrumento, oitava, volume).
4. Sintetizar e tocar a sequência de áudio resultante no dispositivo padrão.
5. Exibir interface gráfica com controles de reprodução.
6. Permitir Iniciar, Pausar e Reiniciar a reprodução.
7. Validar entrada e fornecer feedback sobre caracteres inválidos.
8. Manter o estado atual dos parâmetros musicais do sistema.

== Requisitos Funcionais - Fase 2 (Atualizados)
1. Área de edição de texto para visualização e edição direta.
2. Carregamento de arquivos `.txt` e gravação do texto modificado de volta ao disco.
3. Tratamento de cada linha de texto como uma voz musical independente ($V_0, V_1, V_2, dots$) executadas de forma polifônica e simultânea.
4. Mapeamento de caracteres locais para regras de notas, volumes, instrumentos e oitavas por voz.
5. Suporte a atrasos iniciais por linha usando o formato `[n]`.
6. Alteração global de BPM em tempo real durante a reprodução via símbolos `>` e `<`.
7. Exportação da partitura processada para arquivo MIDI padrão (`.mid`) multilhas.
8. Síntese com FluidSynth e SoundFont em tempo real.
9. Execução da síntese de som de forma assíncrona em thread dedicada para manter a GUI responsiva.
10. Controles visuais de Play, Pause, Stop e volume mestre.

== Requisitos Não Funcionais
1. Aderência rígida a princípios de design orientado a objetos (SOLID).
2. Uso de linguagem C++ padrão 26 para desenvolvimento principal.
3. Interface gráfica desenvolvida com a biblioteca Qt6 e Qt Designer.
4. Gerenciamento e compilação do projeto multiplataforma via Xmake.
5. Controle de versão via Git.
6. Cobertura de regras por meio de testes unitários automatizados gerenciados pelo CTest e Doctest.

= Definições de Classes

#figure(
  image("images/RevisedUML.svg")
)

Abaixo constam as classes que compõem o sistema com seus respectivos atributos encapsulados (visibilidade privada) e métodos públicos após a aplicação das melhorias de design.

// Função auxiliar para padronizar as grades das classes
#let class-grid(columns: (auto, 1fr), ..items) = grid(
  columns: columns,
  column-gutter: 1.5em,
  row-gutter: 1em,
  ..items
)

= Definições de Classes

Abaixo constam as classes que compõem o sistema com seus respectivos atributos encapsulados (visibilidade privada) e métodos públicos após a aplicação das melhorias de design.

== Classe Voice
Representa a trilha/voz melódica polifônica.

*Atributos privados*:
#class-grid(
  raw("currentOctave_: int", lang: "cpp"),
  [Oitava local da voz (0-9)],
  raw("currentVolume_: int", lang: "cpp"),
  [Volume local da voz (0-127)],
  raw("currentInstrument_: int", lang: "cpp"),
  [Código de instrumento General MIDI ativo],
  raw("entryDelayBeats_: int", lang: "cpp"),
  [Silêncio inicial acumulado da voz],
  raw("channel_: int", lang: "cpp"),
  [Canal MIDI físico da voz (pulando canal 9)],
  raw("lastNotePitch_: int", lang: "cpp"),
  [Tom da última nota executada],
  raw("lastWasNote_: bool", lang: "cpp"),
  [Sinalizador se o último evento foi nota],
  raw("currentBeat_: double", lang: "cpp"),
  [Tempo acumulado do cursor na trilha],
  raw("eventQueue_: std::queue<MidiEvent>", lang: "cpp"),
  [Fila de eventos musicais agendados],
)

*Métodos públicos*:
#class-grid(
  raw("Voice(index: int)", lang: "cpp"),
  [Construtor principal aplicando as propriedades iniciais de voz da fuga.],
  raw("applyFugueDefaults(index: int): void", lang: "cpp"),
  [Configura oitava, volume e canal padrão com base no índice.],
  raw("enqueueNote(pitch: int): void", lang: "cpp"),
  [Enfileira eventos de ligar/desligar nota física.],
  raw("enqueueNote(noteName: char): void", lang: "cpp"),
  [Resolve tom dinâmico baseado na oitava atual e enfileira.],
  raw("enqueueNote(noteName: string): void", lang: "cpp"),
  [Resolve tom de nota com alteração (ex: Eb) e enfileira.],
  raw("enqueueEvent(event: MidiEvent): void", lang: "cpp"),
  [Insere um evento genérico na fila.],
  raw("emitBpmChange(bpm: int): void", lang: "cpp"),
  [Adiciona evento de mudança global de BPM.],
  raw("emitInitialProgramChange(): void", lang: "cpp"),
  [Emite mudança de instrumento padrão inicial no instante 0.],
  raw("changeInstrument(instrument: int): void", lang: "cpp"),
  [Executa a troca de instrumento e enfileira o evento associado.],
  raw("changeVolume(volume: int): void", lang: "cpp"),
  [Ajusta o volume interno (clamping) e enfileira evento.],
  raw("doubleVolume(): void", lang: "cpp"),
  [Dobra o volume corrente respeitando os limites MIDI.],
  raw("incrementOctaveOrReset(): void", lang: "cpp"),
  [Sobe oitava ou retorna à oitava padrão.],
  raw("decrementOctave(): void", lang: "cpp"),
  [Diminui oitava local.],
  raw("enqueueRest(): void", lang: "cpp"),
  [Insere tempo de pausa/silêncio.],
  raw("repeatLastNoteOrRest(): void", lang: "cpp"),
  [Repete a última nota caso aplicável, senão insere silêncio.],
)

== Classe IMusicFileService / MusicFileService
Interface e implementação de serviço para isolamento de operações de persistência e conversões externas de acordo com o princípio SRP.

*Métodos públicos*:
#class-grid(
  columns: (1.5fr, 1fr),
  raw("readTextFile(filePath: QString, success: bool&, errorMessage: QString&): QString", lang: "cpp"),
  [Lê um arquivo textual contendo partituras.],
  raw("writeTextFile(filePath: QString, content: QString, errorMessage: QString&): bool", lang: "cpp"),
  [Salva o texto corrente da interface em disco.],
  raw(
    "exportMidiFile(filePath: QString, text: string, initialBpm: int, defaultInstrument: int, errorMessage: QString&): bool",
    lang: "cpp",
  ),
  [Invoca o parser e o gravador MIDI salvando a partitura multilhas convertida para arquivo físico.],
)

== Interface ITextParser / Classe TextParser
Responsável pela interpretação sintática e semântica do texto.

*Atributos privados*:
#class-grid(
  raw("defaultInstrument_: int", lang: "cpp"),
  [Instrumento inicial selecionado no widget principal],
  raw("charRules_: std::unordered_map<char, RuleFunction>", lang: "cpp"),
  [Dicionário de regras mapeando caracteres para ações nas vozes],
)

*Métodos públicos*:
#class-grid(
  columns: (1fr, 1fr),
  raw("parse(text: string, initialBpm: int): std::vector<Voice>", lang: "cpp"),
  [Realiza a decomposição em linhas e constrói a coleção de vozes configuradas.],
  raw("parseDelay(line: string, pos: size_t&): int", lang: "cpp"),
  [Varre a marcação `[n]` retornando o valor do atraso de início.],
  raw("processCharacter(character: char, voice: Voice&): void", lang: "cpp"),
  [Despacha a regra associada ao caractere sobre a instância da voz.],
  raw("setDefaultInstrument(instrument: int): void", lang: "cpp"),
  [Armazena o instrumento mestre inicial.],
)

== Classe MusicMachine (MainWindow)
Controlador gráfico que gerencia os widgets de interface e delega comandos de reprodução e persistência.

*Atributos privados*:
#class-grid(
  raw("ui: Ui::MusicMachine*", lang: "cpp"),
  [Referência para interface visual carregada do XML],
  raw("audioEngine_: std::unique_ptr<AudioEngine>", lang: "cpp"),
  [Motor FluidSynth],
  raw("midiPlayer_: std::unique_ptr<MidiPlayer>", lang: "cpp"),
  [Motor de playback assíncrono de eventos],
  raw("fileService_: std::unique_ptr<IMusicFileService>", lang: "cpp"),
  [Serviço de manipulação física de arquivos (DIP)],
  raw("soundfontPath_: QString", lang: "cpp"),
  [Caminho do banco de áudio carregado],
)

*Métodos públicos*:
#class-grid(
  columns: (65%, 35%),
  raw("MusicMachine(parent: QWidget*, fileService: std::unique_ptr<IMusicFileService>)", lang: "cpp"),
  [Injeta opcionalmente o serviço.],
  raw("getFileDialogPath(saveMode: bool, title: QString, filter: QString): QString", lang: "cpp"),
  [Método auxiliar que unifica diálogos de busca e gravação física de arquivos.],
  raw(
    "onPlayClicked(), onResetClicked(), onOpenClicked(), onSaveClicked(), onExportMidiClicked(), onVolumeChanged(int)",
    lang: "cpp",
  ),
  [Slots de eventos de controle de botões e seletores.],
)

== Outras Classes Auxiliares:

#class-grid(
  raw("AudioEngine", lang: "cpp"),
  [Interface concreta para FluidSynth (```cpp noteOn```, ```cpp noteOff```, ```cpp programChange```, ```cpp volume```).],
  raw("MidiPlayer", lang: "cpp"),
  [Temporizador multithread responsável por consumir as filas de eventos em tempo real.],
  raw("MidiWriter", lang: "cpp"),
  [Conversor de listas de eventos musicais em blocos estruturados da biblioteca ```cpp libremidi```.],
)

= Interface Gráfica (GUI)

== Croqui

#figure(
  image("/images/croqui.png", width: 85%),
  caption: [Croqui],
)
A interface gráfica foi desenhada com foco em clareza, possuindo _menubar_ e _toolbar_. Como é costumeiro em programas com esses elementos de interface, todas as ferramentas da aplicação se encontram na _menubar_, e apenas as mais utilizadas na _toolbar_

#figure(
  box(
    stroke: 1pt + gray,
    radius: 5pt,
    clip: true,
    image("/images/mainWindow.png", width: 85%),
  ),
  caption: [Interface Gráfica Principal],
)
#figure(
  box(
    stroke: 1pt + gray,
    radius: 5pt,
    clip: true,
    image("images/about.png", width: 32%, scaling: "smooth"),
  ),
  caption: [Tela de autoria do código],
)

== Justificativa do Layout
1. *Barra de Ferramentas Superior*: Centraliza os seletores numéricos de andamento (BPM) e instrumentos, facilitando o ajuste de parâmetros.
2. *Área Textual Central*: Um campo rico de edição simples (```cpp QPlainTextEdit```) que permite editar o texto no formato de linhas que representam vozes individuais.
3. *Painel Lateral e Botões*: Unifica o controle imediato de Play/Pause e Parar (Reset), silenciando imediatamente as notas e evitando que canais fiquem ativos após a interrupção.
4. *Menu Help / About*: Apresenta informações detalhadas da versão da aplicação e dos autores.

= Procedimento de Teste

Os arquivos de teste se encontram no repositório.

== Testes Unitários Automatizados (`tests/test_backend.cpp`)
Os testes unitários utilizam o framework `doctest` integrado ao gerenciador do Xmake. A suíte executa 11 casos de testes cobrendo 144 asserções, abrangendo:
1. *Mapeamento de Notas*: Validação da conversão correta de letras (`A-H`) para pitches correspondentes sob diversas oitavas, e testes com clamping fora de escala (0 a 127).
2. *Notas com Acidentes*: Correção do parse de compostos como `Eb`, `Ab`, `Mb`.
3. *Atrasos e Polyphony*: Garantia de que o parser gera o tempo exato de silêncio inicial (`[n]`) em trilhas de múltiplas vozes concorrentes.
4. *Controles de Volume e Instrumentos*: Validação do aumento de volume físico e troca de patch General MIDI baseado em dígitos, vogais e marcações especiais (`!`, `,`, `;`).
5. *Aceleração Global de BPM*: Testes programáticos do sincronismo dinâmico nos comandos `>` e `<`.
6. *Controle de Repetições*: Validação da duplicação da última nota por caracteres não reconhecidos.

== Testes Manuais com Arquivos Estruturados
Adicionalmente, arquivos padrões de testes são fornecidos sob o diretório `tests/` para validação e escuta via GUI:
- `fuga_bwv847.txt` e `fuga_bwv578.txt`: Para testar polifonia clássica de Bach.
- `fuga_bpm.txt`: Modificação dinâmica de andamento durante reprodução.
- `fuga_instrumentos.txt`: Alterações sucessivas de canais e patches.

= Relatório de Revisão de Código e Ações Corretivas

Abaixo, detalhamos cada ponto identificado pelo Grupo Revisor 4 em seu relatório e as respectivas ações que implementamos na estrutura do sistema:

== A. Princípio de Responsabilidade Única (SRP) e Inversão de Dependências (DIP) na GUI
- *Problema Apontado*: A classe visual `MusicMachine` (MainWindow) acumulava responsabilidades de leitura e gravação no disco (```cpp QFile```/```cpp QTextStream```) e geração de MIDI, acoplando a lógica com componentes de tela.
- *Ação Corretiva*: Criamos a interface abstrata ```cpp IMusicFileService``` e a implementação concreta ```cpp MusicFileService```. Toda a manipulação de leitura, gravação e o encadeamento de exportação MIDI do ```cpp TextParser``` com o ```cpp MidiWriter``` foram extraídos para o serviço. O controlador visual apenas requisita a persistência via interface, permitindo mock de testes e desacoplamento completo.

== B. Encapsulamento de Atributos do Modelo `Voice`
- *Problema Apontado*: Variáveis e estados fundamentais (como oitavas locais e volume corrente) estavam declarados de forma pública.
- *Ação Corretiva*: Tornamos todos os campos de controle internos privados no cabeçalho `Voice.h` e expusemos métodos getters públicos limpos (```cpp getCurrentOctave()```, ```cpp getCurrentVolume()```, etc.), mantendo a integridade dos limites e consistência do objeto.

== C. Inveja de Dados (Feature Envy) em ```cpp TextParser```
- *Problema Apontado*: A classe de parsing continha fórmulas e mutações diretas sobre os valores internos de ```cpp Voice``` (ex: ```cpp voice.currentVolume = std::min(127, voice.currentVolume * 2)```), violando as boas práticas de encapsulamento comportamental.
- *Ação Corretiva*: Adicionamos métodos de ação com regras explícitas na classe ```cpp Voice```, como ```cpp doubleVolume()```, ```cpp changeInstrument(int)```, ```cpp incrementOctaveOrReset()```, e ```cpp decrementOctave()```. O processador de texto agora se limita a disparar esses métodos sob a voz correspondente, deixando que a voz gerencie sua matemática interna.

== D. Eliminação de Código Duplicado na Interface
- *Problema Apontado*: Códigos estruturalmente idênticos para configuração de caixas de diálogo e tratamento de fluxos de erros físicos em ```cpp onOpenClicked()```, ```cpp onSaveClicked()``` e ```cpp onExportMidiClicked()```.
- *Ação Corretiva*: Implementamos o método centralizado ```cpp getFileDialogPath(bool saveMode, const QString& title, const QString& filter)``` que unifica a abertura de diálogos do Qt6, reduzindo a duplicação e facilitando o gerenciamento centralizado de erros de cancelamento ou leitura.

== E. Eliminação de Números Mágicos na Conversão
- *Problema Apontado*: As conversões de semitons manuais (`+ 3` e `+ 8`) no motor de pitch MIDI eram representadas por inteiros literais jogados no meio das expressões matemáticas.
- *Ação Corretiva*: Definimos constantes estáticas explicativas (```cpp SEMITONE_E_FLAT = 3``` e ```cpp SEMITONE_A_FLAT = 8```) no modelo para documentar e organizar os cálculos matemáticos.

== Números Mágicos na classe MidiWriter
Enquanto mostrávamos o código para o professor, também indentificamos que a classe ```cpp MidiWriter``` poderia ter sua qualidade interna melhorada, assim:
- *Problema*: O arquivo MidiWriter.cpp continha diversos números mágicos associados a bytes status do protocolo MIDI (```cpp 0x90, 0x80, 0xC0, 0xB0```), máscaras de canal (```cpp 0x0F```) e índice do controlador de volume (```cpp 0x7F```). Além de constantes de tempo de andamento (```cpp 60000000, 120```).
- *Ação Corretiva*: Foram criadas constantes simbólicas descritivas em um namespace anônimo no arquivo MidiWriter.cpp (```cpp MIDI_STATUS_NOTE_ON, MIDI_CHANNEL_MASK, MIDI_CC_VOLUME, MICROSECONDS_PER_MINUTE```), eliminando os valores numéricos literais e tornando a representação do protocolo MIDI autodescritiva.

= Conclusão

Com a aplicação destas modificações baseadas no relatório de revisão de código da Fase 3, o projeto atingiu um nível de maturidade superior, garantindo o isolamento da lógica de negócios em relação à interface visual, a proteção do estado interno das vozes e a eliminação de redundâncias, respeitando inteiramente os princípios de design de software orientado a objetos SOLID e as especificações.

