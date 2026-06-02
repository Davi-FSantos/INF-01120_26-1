#set page(
  width: 21cm,
  height: auto,
)

#v(1em)
#align(center, text(size: 2em, weight: "bold")[Relatório de Desenvolvimento - Fase 2])
#v(1em)

#text(size: 1.2em)[Desenvolvimento de Software #h(1fr) INF01120] \
#text(size: 1.2em)[#underline[Davi Santos] #h(1fr) #underline[00599530]] \
#text(size: 1.2em)[#underline[Gabriel Schons] #h(1fr) #underline[00342020]] \
#text(size: 1.2em)[#underline[Gabriel Copatti] #h(1fr) #underline[00342808]] \
#text(size: 1.2em)[#underline[Julio Augusto de Castilhos Borges] #h(1fr) #underline[00590537]]

= Lista de Requisitos Funcionais

O sistema deve:

1. Fornecer uma área de edição de texto na interface gráfica para que o usuário possa visualizar, editar e inserir sequências de texto livremente.

2. Permitir carregar sequências de texto a partir de arquivos no formato texto simples (`.txt`) e salvar o texto editado de volta no mesmo formato.

3. Tratar cada linha do texto de entrada como uma voz musical independente ($V_0, V_1, V_2, dots$), processando-as de forma polifônica e simultânea.

4. Mapear caracteres específicos do texto para eventos musicais locais (notas, volumes locais, instrumentos locais e alterações locais de oitava) de acordo com regras estritas de interpretação.

5. Suportar atrasos iniciais individuais para cada voz através da sintaxe `[n]` inserida no início da linha de texto correspondente, que introduz $n$ tempos de silêncio antes do processamento.

6. Processar comandos de alteração de andamento (`>` e `<`) de escopo global, alterando o BPM do sistema para todas as vozes em tempo real durante a reprodução.

7. Exportar a composição gerada para arquivos de música no formato MIDI padrão (`.mid`), organizando cada voz em trilhas MIDI independentes.

8. Sintetizar e reproduzir o áudio resultante em tempo real utilizando a biblioteca FluidSynth com carregamento de tabelas de ondas do formato SoundFont.

9. Executar a síntese de som de maneira assíncrona em uma thread dedicada separada, impedindo o bloqueio ou congelamento da interface gráfica durante o processamento de áudio.

10. Permitir controlar a execução da música através de comandos de Tocar, Pausar e Parar, além de possibilitar o ajuste de parâmetros globais iniciais na interface.

= Requisitos Não Funcionais

1. O sistema deve seguir rigorosamente os princípios de programação orientada a objetos (SOLID).

2. O código-fonte principal deve ser escrito em C++ com suporte ao padrão C++26.

3. A interface gráfica deve ser construída utilizando a biblioteca Qt6 e arquivos de definição visual do Qt Designer (`.ui`).

4. A síntese sonora em tempo real deve ser realizada através do sintetizador FluidSynth integrado ao código.

5. O ciclo de vida do build da aplicação deve ser configurado via CMake, gerenciando dependências do Qt6 e FluidSynth de maneira reprodutível.

6. O controle de versões do projeto deve ser gerenciado através do sistema Git.

7. Testes unitários de validação da geração de eventos musicais devem ser implementados com suporte do CTest.

= Definição das Classes

== Diagrama UML

#image("/images/UML.svg")

== Classe Voice

*Atributos:*
- `instrument: int` - Armazena o ID do instrumento MIDI ativo para a trilha (0-127)
- `volume: int` - Armazena o volume atual da trilha (0-127)
- `octave: int` - Armazena a oitava local atual (0-9)
- `initialDelay: int` - Armazena a quantidade de batidas de atraso inicial (`[n]`) da voz
- `noteQueue: std::queue<MidiEvent>` - Fila de eventos musicais (notas e silêncios) agendados
- `lastNote: int` - Armazena a última nota MIDI reproduzida (para regras de repetição)
- `isPlaying: bool` - Indica se a voz está sendo ativamente processada

*Métodos:*
- `reset(baseInstrument: int, baseVolume: int, baseOctave: int, delay: int): void` - Restaura o estado interno e parâmetros iniciais da voz
- `setInstrument(instrumentId: int): void` - Modifica o instrumento da voz
- `setVolume(vol: int): void` - Define o volume local
- `setOctave(oct: int): void` - Define a oitava da voz, com limites inferior (0) e superior (9)
- `enqueueNote(note: int, duration: float, volume: int): void` - Adiciona um evento de nota MIDI à fila
- `enqueueSilence(duration: float): void` - Adiciona um evento de pausa à fila de eventos
- `getNextEvent(): MidiEvent` - Retorna e remove o próximo evento MIDI da fila
- `hasEvents(): bool` - Verifica se há eventos pendentes na fila de reprodução

== Classe TokenProcessor

*Atributos:*
- `mappingRules: std::unordered_map<char, Instruction>` - Dicionário que mapeia caracteres para instruções musicais correspondentes

*Métodos:*
- `tokenize(input: string, voices: std::vector<Voice>&): void` - Processa o texto completo de entrada, particionando em linhas e configurando a fila de notas de cada voz
- `parseLine(line: string, voiceIndex: int, voice: Voice&): void` - Analisa lexicalmente uma linha individual de texto, gerando e enfileirando eventos de música na voz correspondente
- `parseDelay(line: string): int` - Analisa o prefixo de atraso `[n]` no início da string, retornando o número de tempos de atraso extraído

== Classe MusicMachine

*Atributos:*
- `voices: std::vector<Voice>` - Vetor contendo a lista de vozes polifônicas gerenciadas
- `globalBPM: int` - Armazena o andamento global em batidas por minuto (BPM)
- `tokenProcessor: TokenProcessor` - Referência para a classe de interpretação de texto
- `audioEngine: AudioEngine` - Referência para a classe gerenciadora do motor de síntese

*Métodos:*
- `initialize(text: string): void` - Prepara a máquina para reprodução realizando o parsing do texto e inicializando as vozes
- `play(): void` - Inicia a reprodução polifônica concorrente de todas as vozes
- `pause(): void` - Pausa temporariamente a execução global
- `stop(): void` - Finaliza a reprodução e reseta o cursor de eventos
- `setBPM(bpm: int): void` - Define o BPM global inicial
- `adjustBPM(delta: int): void` - Ajusta o BPM global dinamicamente durante a reprodução
- `exportToMidi(filepath: string): bool` - Exporta a sequência de eventos de todas as vozes para um arquivo padrão `.mid`

== Classe AudioEngine

*Atributos:*
- `settings: fluid_settings_t*` - Configurações internas de alocação do sintetizador FluidSynth
- `synth: fluid_synth_t*` - Instância ativa do gerador de síntese FluidSynth
- `adriver: fluid_audio_driver_t*` - Driver de saída de áudio de tempo real do sistema operacional
- `soundfontId: int` - ID identificador do arquivo de SoundFont (`.sf2`) carregado
- `isPlaying: bool` - Estado atual da síntese sonora

*Métodos:*
- `initialize(sfPath: string): bool` - Configura o motor, carrega a SoundFont e ativa o driver de som
- `noteOn(channel: int, key: int, velocity: int): void` - Envia instrução MIDI para tocar nota no sintetizador
- `noteOff(channel: int, key: int): void` - Envia instrução MIDI para silenciar nota
- `programChange(channel: int, program: int): void` - Associa um instrumento MIDI ao canal informado
- `setChannelVolume(channel: int, volume: int): void` - Ajusta o volume do canal de síntese específico
- `shutdown(): void` - Para a síntese e desaloca recursos de memória de áudio

== Classe MidiWriter

*Atributos:*
- `writer_: libremidi::writer` - Instância de controle de escrita de arquivos MIDI polifônicos
- `TICKS_PER_BEAT: int` - Resolução de tempo (constante, padrão 480 ticks/beat)

*Métodos:*
- `createFile(): void` - Inicializa as estruturas necessárias para o arquivo MIDI multi-track
- `writeVoiceTrack(trackIndex: int, voice: Voice): void` - Converte a fila de eventos de uma voz em eventos MIDI e grava em sua trilha dedicada
- `save(filepath: string): bool` - Salva a estrutura criada em disco no formato binário `.mid`

== Classe MainWindow

*Atributos:*
- `musicMachine: MusicMachine` - Instância da lógica central controladora da máquina
- `textEditor: QTextEdit*` - Componente visual da interface gráfica de edição de texto
- `playButton: QPushButton*` - Botão para disparar a execução
- `pauseButton: QPushButton*` - Botão para pausar a execução
- `stopButton: QPushButton*` - Botão para parar e resetar o andamento
- `bpmSpinBox: QSpinBox*` - Seletor numérico para configuração de BPM
- `exportButton: QPushButton*` - Botão para salvar arquivo de música
- `fileMenu: QMenu*` - Menu superior de opções de abertura/gravação de arquivos

*Métodos:*
- `setupUI(): void` - Liga e conecta dinamicamente os widgets do arquivo `.ui` às funções slots de controle
- `onPlayClicked(): void` - Inicializa a reprodução assíncrona
- `onPauseClicked(): void` - Pausa o thread de áudio
- `onStopClicked(): void` - Envia comando de interrupção e reinício de canais
- `onExportClicked(): void` - Solicita caminho de gravação e delega exportação
- `onOpenFile(): void` - Importa arquivos `.txt` externos para o painel de texto
- `onSaveFile(): void` - Salva o texto editado pelo painel para disco

= GUI em Qt6

#image("/images/qtmidi.png")

= Arquitetura do Sistema

O sistema é estruturado em uma arquitetura modular orientada a objetos projetada para suportar a reprodução e processamento de áudio polifônico em tempo real, sem bloquear ou degradar a responsividade da interface com usuário. O design separa estritamente as seguintes responsabilidades:

- *Camada de Apresentação (GUI)*: Representada pela classe `MainWindow`, consome a interface de usuário definida no arquivo `qtmidi.ui` gerado pelo Qt Designer. Esta camada reage a eventos e ações do usuário (carregar/salvar texto, tocar/pausar áudio, configurar parâmetros) e delega as atualizações e disparos para a camada lógica. Toda a manipulação de som e a síntese ocorrem de forma assíncrona (usando threads internas de temporização ou processamento assíncrono), prevenindo o travamento da GUI durante o tempo de execução da reprodução.

- *Camada Lógica Principal*: Coordenada pela classe `MusicMachine`, que orquestra a leitura e execução do processador de texto, controle das vozes polifônicas e sincronismo global. A classe `Voice` encapsula o estado melódico independente de cada trilha de áudio (canal MIDI dedicado, oitava local, volume local, instrumento local e fila de eventos), fornecendo uma representação independente dos dados melódicos das trilhas.

- *Camada de Análise Léxica e Semântica*: Implementada pela classe `TokenProcessor`, ela interpreta a entrada textual estruturada em múltiplas linhas. Ela faz o parsing de regras locais (notas A-H, pausas, modulação de oitava/volume/instrumento por caractere) de forma independente para preencher a fila de eventos de cada instância de `Voice`. Ela também reconhece metadados globais (como aceleração de BPM por `>` ou `<`) e inicializações especiais (como o atraso de voz `[n]`).

- *Camada de Síntese e Driver de Áudio*: A classe `AudioEngine` isola a biblioteca externa FluidSynth do restante da lógica da aplicação. Ela realiza a inicialização assíncrona dos buffers, carregamento da SoundFont e despacho de eventos MIDI brutos em canais mapeados para cada voz do sistema. Isso permite que a biblioteca de áudio ou sintetizador físico seja substituído no futuro por outra tecnologia compatível sem que a lógica central da aplicação precise ser reescrita (atendendo ao princípio de Inversão de Dependência).

- *Camada de Exportação*: A classe `MidiWriter` encapsula o empacotamento em formato de trilhas de arquivo MIDI padrão, gravando em disco o resultado estático da interpretação musical.

- *Refinamentos de Implementação*: Durante o desenvolvimento do código-fonte, a classe conceitual `MusicMachine` descrita na modelagem foi refinada em `MidiPlayer` (responsável exclusiva pela gerência da thread assíncrona de reprodução e controle refinado de tempo) e em `MainWindow` (gerenciando a janela gráfica e os slots de interação), garantindo maior coesão de acordo com o Princípio de Responsabilidade Única (SRP). Adicionalmente, o componente de análise conceitual `TokenProcessor` foi implementado como a classe `TextParser`, a qual implementa a interface extensível `ITextParser` (promovendo desacoplamento e facilitando a substituição de regras sem alterar as classes consumidoras, em conformidade com o Princípio Aberto/Fechado - OCP).

= Suposições de Projeto

Para o correto funcionamento do sistema e delimitação do escopo técnico na Fase 2, foram estabelecidas as seguintes suposições de projeto:

1. *Disponibilidade de SoundFonts*: O motor de síntese FluidSynth exige um arquivo de SoundFont (`.sf2` ou `.sf3`) General MIDI para gerar áudio analógico. Assume-se que o ambiente operacional possui caminhos comuns de bibliotecas de áudio populados (como `/usr/share/sounds/sf2/FluidR3_GM.sf2` no Linux). Como contingência, o sistema provê um diálogo interativo de seleção de arquivos (`QFileDialog`) caso nenhuma SoundFont padrão seja encontrada automaticamente, permitindo ao usuário indicar o caminho do arquivo manualmente.
2. *Reserva do Canal MIDI 9*: Por especificação do protocolo General MIDI, o canal 10 (indexado como 9) é estritamente dedicado a instrumentos de percussão (bateria). Para evitar timbres ruidosos e incoerentes ao enviar notas e programações melódicas para esse canal, o sistema assume que o canal 9 deve ser explicitamente ignorado durante o mapeamento de notas melódicas e alterações gerais de timbres.
3. *Tratamento de Andamento Global*: Supõe-se que as instruções de aceleração (`>`) e desaceleração (`<`) possuem escopo global. Elas afetam a taxa de tempo (BPM) compartilhada por todas as vozes concorrentes em tempo de execução, mudando a duração física de cada pulso em tempo real sem alterar a contagem lógica do atraso inicial de cada voz. O andamento mínimo é travado em 40 BPM para impedir erros de divisão por zero ou congelamento temporal do sequenciador.
4. *Escopo de Parâmetros Locais*: Volume, oitava (`?`/`V`/`.`) e instrumento são considerados locais de cada voz e processados individualmente em canais MIDI separados, garantindo que a execução polifônica preserve as dinâmicas particulares de cada linha textual.

= Justificativa da Interface Gráfica (GUI)

O design visual da interface gráfica foi elaborado sob a premissa de simplicidade operacional, ergonomia e fácil experimentação por parte do usuário. As decisões de layout justificam-se conforme os seguintes pontos:

1. *Área de Edição Textual Livre*: O componente principal é um editor de texto amplo (`QTextEdit`), simulando uma partitura digital onde cada linha do texto corresponde diretamente a uma voz da fuga. O usuário tem liberdade de edição, permitindo copiar, colar e modificar facilmente trechos musicais.
2. *Barra de Parâmetros Iniciais*: Posicionada no topo da interface, permite configurar o andamento inicial (BPM) via seletor numérico (`QSpinBox`), selecionar o timbre padrão da voz principal (`QComboBox`) e controlar o volume geral via seletor deslizante (`QSlider`). Isso centraliza as configurações da reprodução em um local visível e de fácil acesso.
3. *Controles de Reprodução Intuitivos (Play/Pause/Stop)*: Posicionados de forma adjacente, os botões oferecem controle imediato sobre o áudio. O botão Play ativa a reprodução assíncrona, alternando seu estado visual para Pause para pausar temporariamente. O botão Stop interrompe a execução, resetando os ponteiros de eventos e forçando o silenciamento imediato de todas as notas ativas nas vozes para sanar problemas de "notas presas" (hanging notes).
4. *Diálogo de Informações (About)*: Um diálogo dedicado (`AboutDialog`) foi projetado via Qt Designer e pode ser aberto a partir do menu "Help > About". Esta janela apresenta detalhes da versão do sistema, dos integrantes do grupo de desenvolvimento e inclui um botão interativo conectado a `QDesktopServices::openUrl()` que abre o repositório oficial no navegador padrão do sistema operacional, facilitando o acesso ao código de maneira elegante.
5. *Atalhos do Teclado*: O menu superior "Playback" expõe as opções de controle e foca nos respectivos seletores de andamento e instrumentos ao serem selecionados, otimizando o fluxo de trabalho do usuário avançado.

= Descrição do Procedimento de Teste

A validação de corretude do sistema e do motor de áudio assíncrono abrangeu tanto testes unitários e arquivos de testes estruturados quanto testes de verificação manual do comportamento da interface e concorrência:

== Casos de Teste Automatizados e Estruturados
O diretório `tests/` concentra arquivos de texto simples que validam cenários específicos de interpretação semântica e processamento de tokens. O parser realiza a extração e o enfileiramento correspondentes:
1. *Fuga BWV 847 (`tests/fuga_bwv847.txt`)* e *Fuga BWV 578 (`tests/fuga_bwv578.txt`)*: Validação do processamento polifônico complexo, garantindo que múltiplas vozes com atrasos de entrada (`[n]`) rodem concorrentemente sem falhas de sincronia.
2. *Fuga BPM (`tests/fuga_bpm.txt`)*: Testa o processamento dos modificadores globais de andamento (`>` e `<`), garantindo que o BPM do sequenciador seja acelerado e desacelerado de forma homogênea para todas as trilhas ativas.
3. *Fuga Instrumentos (`tests/fuga_instrumentos.txt`)*: Valida se modificadores como dígitos e caracteres especiais (`!`, `;`, `,`, vogais) resultam nas corretas trocas de canais de instrumentos do sintetizador FluidSynth.
4. *Fuga Pausas (`tests/fuga_pausas.txt`)*: Valida o comportamento de silenciamento temporário introduzido por caracteres minúsculos (`a-h`) e outras consoantes.
5. *Fuga Oitavas (`tests/fuga_oitavas.txt`)*: Confirma que os comandos `?`, `V` e `.` alteram as oitavas locais nos limites definidos (0 a 9) e que o estouro superior reinicia à oitava de partida.
6. *Fuga Repetidas (`tests/fuga_repetidas.txt`)*: Valida a semântica de repetição de nota ao encontrar consoantes ou caracteres não mapeados logo após uma nota válida.

== Verificação Manual de Concorrência e Estabilidade
- *Teste de Responsividade*: Durante a reprodução contínua de peças polifônicas complexas, o usuário realiza operações na janela principal (digitação de novos textos, redimensionamento de janelas e clique em menus). A interface gráfica mantém-se totalmente fluida e responsiva a 60 FPS, demonstrando que a thread assíncrona secundária isola o processamento pesado de áudio sem congelar o loop principal de eventos do Qt.
- *Teste de Notas Presas (Hanging Notes)*: Com a música tocando no meio de passagens com notas de longa sustentação, o usuário pressiona "Stop" ou "Pause". Confirma-se visualmente e auditivamente que todas as notas são silenciadas no mesmo milissegundo, validando o gerenciamento thread-safe do vetor de notas ativas do `MidiPlayer`.
- *Teste de Fallback de SoundFont*: Ao inicializar o executável em uma máquina sem fontes do sintetizador de áudio, confirma-se o surgimento de uma caixa de aviso indicando a ausência do SoundFont, seguida pelo seletor de arquivos local `QFileDialog`. Ao escolher uma SoundFont externa, a síntese de áudio é restabelecida imediatamente.
