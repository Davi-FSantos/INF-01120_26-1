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
- ```cpp instrument: int``` - Armazena o ID do instrumento MIDI ativo para a trilha (0-127)
- ```cpp volume: int``` - Armazena o volume atual da trilha (0-127)
- ```cpp octave: int``` - Armazena a oitava local atual (0-9)
- ```cpp initialDelay: int``` - Armazena a quantidade de batidas de atraso inicial (```cpp [n]```) da voz
- ```cpp noteQueue: std::queue<MidiEvent>``` - Fila de eventos musicais (notas e silêncios) agendados
- ```cpp lastNote: int``` - Armazena a última nota MIDI reproduzida (para regras de repetição)
- ```cpp isPlaying: bool``` - Indica se a voz está sendo ativamente processada

*Métodos:*
- ```cpp reset(baseInstrument: int, baseVolume: int, baseOctave: int, delay: int): void``` - Restaura o estado interno e parâmetros iniciais da voz
- ```cpp setInstrument(instrumentId: int): void``` - Modifica o instrumento da voz
- ```cpp setVolume(vol: int): void``` - Define o volume local
- ```cpp setOctave(oct: int): void``` - Define a oitava da voz, com limites inferior (0) e superior (9)
- ```cpp enqueueNote(note: int, duration: float, volume: int): void``` - Adiciona um evento de nota MIDI à fila
- ```cpp enqueueSilence(duration: float): void``` - Adiciona um evento de pausa à fila de eventos
- ```cpp getNextEvent(): MidiEvent``` - Retorna e remove o próximo evento MIDI da fila
- ```cpp hasEvents(): bool``` - Verifica se há eventos pendentes na fila de reprodução

== Classe TokenProcessor

*Atributos:*
- ```cpp mappingRules: std::unordered_map<char, Instruction>``` - Dicionário que mapeia caracteres para instruções musicais correspondentes

*Métodos:*
- ```cpp tokenize(input: string, voices: std::vector<Voice>&): void``` - Processa o texto completo de entrada, particionando em linhas e configurando a fila de notas de cada voz
- ```cpp parseLine(line: string, voiceIndex: int, voice: Voice&): void``` - Analisa lexicalmente uma linha individual de texto, gerando e enfileirando eventos de música na voz correspondente
- ```cpp parseDelay(line: string): int``` - Analisa o prefixo de atraso (```cpp [n]```) no início da string, retornando o número de tempos de atraso extraído

== Classe MusicMachine

*Atributos:*
- ```cpp voices: std::vector<Voice>``` - Vetor contendo a lista de vozes polifônicas gerenciadas
- ```cpp globalBPM: int``` - Armazena o andamento global em batidas por minuto (BPM)
- ```cpp tokenProcessor: TokenProcessor``` - Referência para a classe de interpretação de texto
- ```cpp audioEngine: AudioEngine``` - Referência para a classe gerenciadora do motor de síntese

*Métodos:*
- ```cpp initialize(text: string): void``` - Prepara a máquina para reprodução realizando o parsing do texto e inicializando as vozes
- ```cpp play(): void``` - Inicia a reprodução polifônica concorrente de todas as vozes
- ```cpp pause(): void``` - Pausa temporariamente a execução global
- ```cpp stop(): void``` - Finaliza a reprodução e reseta o cursor de eventos
- ```cpp setBPM(bpm: int): void``` - Define o BPM global inicial
- ```cpp adjustBPM(delta: int): void``` - Ajusta o BPM global dinamicamente durante a reprodução
- ```cpp exportToMidi(filepath: string): bool``` - Exporta a sequência de eventos de todas as vozes para um arquivo padrão (```cpp .mid```)

== Classe AudioEngine

*Atributos:*
- ```cpp settings: fluid_settings_t*``` - Configurações internas de alocação do sintetizador FluidSynth
- ```cpp synth: fluid_synth_t*``` - Instância ativa do gerador de síntese FluidSynth
- ```cpp adriver: fluid_audio_driver_t*``` - Driver de saída de áudio de tempo real do sistema operacional
- ```cpp soundfontId: int``` - ID identificador do arquivo de SoundFont (```cpp .sf2```) carregado
- ```cpp isPlaying: bool``` - Estado atual da síntese sonora

*Métodos:*
- ```cpp initialize(sfPath: string): bool``` - Configura o motor, carrega a SoundFont e ativa o driver de som
- ```cpp noteOn(channel: int, key: int, velocity: int): void``` - Envia instrução MIDI para tocar nota no sintetizador
- ```cpp noteOff(channel: int, key: int): void``` - Envia instrução MIDI para silenciar nota
- ```cpp programChange(channel: int, program: int): void``` - Associa um instrumento MIDI ao canal informado
- ```cpp setChannelVolume(channel: int, volume: int): void``` - Ajusta o volume do canal de síntese específico
- ```cpp shutdown(): void``` - Para a síntese e desaloca recursos de memória de áudio

== Classe MidiWriter

*Atributos:*
- ```cpp writer_: libremidi::writer``` - Instância de controle de escrita de arquivos MIDI polifônicos
- ```cpp TICKS_PER_BEAT: int``` - Resolução de tempo (constante, padrão 480 ticks/beat)

*Métodos:*
- ```cpp createFile(): void``` - Inicializa as estruturas necessárias para o arquivo MIDI multi-track
- ```cpp writeVoiceTrack(trackIndex: int, voice: Voice): void``` - Converte a fila de eventos de uma voz em eventos MIDI e grava em sua trilha dedicada
- ```cpp save(filepath: string): bool``` - Salva a estrutura criada em disco no formato binário (```cpp .mid```)

== Classe MainWindow

*Atributos:*
- ```cpp musicMachine: MusicMachine``` - Instância da lógica central controladora da máquina
- ```cpp textEditor: QTextEdit*``` - Componente visual da interface gráfica de edição de texto
- ```cpp playButton: QPushButton*``` - Botão para disparar a execução
- ```cpp pauseButton: QPushButton*``` - Botão para pausar a execução
- ```cpp stopButton: QPushButton*``` - Botão para parar e resetar o andamento
- ```cpp bpmSpinBox: QSpinBox*``` - Seletor numérico para configuração de BPM
- ```cpp exportButton: QPushButton*``` - Botão para salvar arquivo de música
- ```cpp fileMenu: QMenu*``` - Menu superior de opções de abertura/gravação de arquivos

*Métodos:*
- ```cpp setupUI(): void``` - Liga e conecta dinamicamente os widgets do arquivo (```cpp .ui```) às funções slots de controle
- ```cpp onPlayClicked(): void``` - Inicializa a reprodução assíncrona
- ```cpp onPauseClicked(): void``` - Pausa o thread de áudio
- ```cpp onStopClicked(): void``` - Envia comando de interrupção e reinício de canais
- ```cpp onExportClicked(): void``` - Solicita caminho de gravação e delega exportação
- ```cpp onOpenFile(): void``` - Importa arquivos (```cpp .txt```) externos para o painel de texto
- ```cpp onSaveFile(): void``` - Salva o texto editado pelo painel para disco

= GUI em Qt6

#image("/images/musicMachine.png")

= Arquitetura do Sistema

O sistema é estruturado em uma arquitetura modular orientada a objetos projetada para suportar a reprodução e processamento de áudio polifônico em tempo real, sem bloquear ou degradar a responsividade da interface com usuário. O design separa estritamente as seguintes responsabilidades:

- *Camada de Apresentação (GUI)*: Representada pela classe ```cpp MainWindow```, consome a interface de usuário definida no arquivo ```cpp musicMachine.ui``` gerado pelo Qt Designer visando evitar a definição de interfaces de usuário por código-fonte. Esta camada reage a eventos e ações do usuário (carregar/salvar texto, tocar/pausar áudio, configurar parâmetros) e delega as atualizações e disparos para a camada lógica. Toda a manipulação de som e a síntese ocorrem de forma assíncrona (usando threads internas de temporização ou processamento assíncrono).

- *Camada Lógica Principal*: Coordenada pela classe ```cpp MusicMachine```, que orquestra a leitura e execução do processador de texto, controle das vozes polifônicas e sincronismo global. A classe ```cpp Voice``` encapsula o estado melódico independente de cada trilha de áudio (canal MIDI dedicado, oitava local, volume local, instrumento local e fila de eventos), fornecendo uma representação independente dos dados melódicos das trilhas.

- *Camada de Análise Léxica e Semântica*: Implementada pela classe ```cpp TokenProcessor```, ela interpreta a entrada textual estruturada em múltiplas linhas. Também interpreta as regras locais (notas A-H, pausas, modulação de oitava/volume/instrumento por caractere) de forma independente para preencher a fila de eventos de cada instância de ```cpp Voice```. Ela também reconhece metadados globais (como aceleração de BPM por ```cpp >``` ou ```cpp <```) e inicializações especiais (como o atraso de voz ```cpp [n]```).

- *Camada de Síntese e Driver de Áudio*: A classe ```cpp AudioEngine``` isola a biblioteca externa FluidSynth do restante da lógica da aplicação. Ela realiza a inicialização assíncrona dos buffers, carregamento da SoundFont e despacho de eventos MIDI brutos em canais mapeados para cada voz do sistema. Isso permite que a biblioteca de áudio ou sintetizador físico seja substituído no futuro por outra tecnologia compatível sem que a lógica central da aplicação precise ser reescrita (atendendo ao princípio de Inversão de Dependência).

- *Camada de Exportação*: A classe ```cpp MidiWriter``` encapsula o empacotamento em formato de trilhas de arquivo MIDI padrão, gravando em disco o resultado estático da interpretação musical.

- *Refinamentos de Implementação*: Durante o desenvolvimento do código-fonte, a classe conceitual ```cpp MusicMachine``` descrita na modelagem foi refinada em ```cpp MidiPlayer``` (responsável exclusiva pela gerência da thread assíncrona de reprodução e controle refinado de tempo) e em ```cpp MainWindow``` (gerenciando a janela gráfica e os slots de interação), garantindo maior coesão de acordo com o Princípio de Responsabilidade Única (SRP). Adicionalmente, o componente de análise conceitual ```cpp TokenProcessor``` foi implementado como a classe ```cpp TextParser```, a qual implementa a interface extensível ```cpp ITextParser``` (promovendo desacoplamento e facilitando a substituição de regras sem alterar as classes consumidoras, em conformidade com o Princípio Aberto/Fechado - OCP).

= Suposições de Projeto

Para o correto funcionamento do sistema e delimitação do escopo técnico na Fase 2, foram estabelecidas as seguintes suposições de projeto:

1. *Disponibilidade de SoundFonts*: O motor de síntese FluidSynth exige um arquivo de SoundFont (```.sf2``` ou ```.sf3```) General MIDI para gerar áudio analógico. Assume-se que o ambiente operacional possui caminhos comuns de bibliotecas de áudio populados (como ```/usr/share/sounds/sf2/FluidR3_GM.sf2``` no Linux). Como contingência, o sistema provê um diálogo interativo de seleção de arquivos (```cpp QFileDialog```) caso nenhuma SoundFont padrão seja encontrada automaticamente, permitindo ao usuário indicar o caminho do arquivo manualmente.
2. *Reserva do Canal MIDI 9*: Por especificação do protocolo General MIDI, o canal 10 (indexado como 9) é estritamente dedicado a instrumentos de percussão. O sistema desconsidera o canal 9 para evitar problemas de compatibilidade com a sintetização de instrumentos diferentes. Assim, como MIDI 1.0 possui 16 canais, ficamos com 15 restantes, o que é mais do que o suficiente para o escopo desta aplicação. Além disso, não existe uma limitação de 15 'linhas', pois continuamos 're'mapeando linhas acima de 15 de volta para os canais iniciais. $n % 15$, $n$ sendo número de linhas. No entanto, isso significa que mudanças de instrumento e volume em qualquer linha remapeada ao canal serão refletidas em todas as linhas que compartilham aquele canal.
3. *Restrições de BPM*: As instruções de aceleração (`>`) e desaceleração (`<`) possuem escopo global, como diz a especificação. No entanto, restringimos os valores máximo e mínimo para 1200 e 40, respecivamente, pois foi o valor que uma pesquisa rápida retornou como limites aceitáveis de BPM.

// = Justificativa da Interface Gráfica (GUI)
// 
// O design visual da interface gráfica foi elaborado sob a premissa de simplicidade operacional, ergonomia e fácil experimentação por parte do usuário. As decisões de layout justificam-se conforme os seguintes pontos:
// 
// 1. *Área de Edição Textual Livre*: O componente principal é um editor de texto amplo (```cpp QTextEdit```), simulando uma partitura digital onde cada linha do texto corresponde diretamente a uma voz da fuga. O usuário tem liberdade de edição, permitindo copiar, colar e modificar facilmente trechos musicais.
// 2. *Barra de Parâmetros Iniciais*: Posicionada no topo da interface, permite configurar o andamento inicial (BPM) via seletor numérico (```cpp QSpinBox```), selecionar o timbre padrão da voz principal (```cpp QComboBox```) e controlar o volume geral via seletor deslizante (```cpp QSlider```). Isso centraliza as configurações da reprodução em um local visível e de fácil acesso.
// 3. *Controles de Reprodução Intuitivos (Play/Pause/Stop)*: Posicionados de forma adjacente, os botões oferecem controle imediato sobre o áudio. O botão Play ativa a reprodução assíncrona, alternando seu estado visual para Pause para pausar temporariamente. O botão Stop interrompe a execução, resetando os ponteiros de eventos e forçando o silenciamento imediato de todas as notas ativas nas vozes para sanar problemas de "notas presas" (hanging notes).
// 4. *Diálogo de Informações (About)*: Um diálogo dedicado (```cpp AboutDialog```) foi projetado via Qt Designer e pode ser aberto a partir do menu "Help > About". Esta janela apresenta detalhes da versão do sistema, dos integrantes do grupo de desenvolvimento e inclui um botão interativo conectado a ```cpp QDesktopServices::openUrl()``` que abre o repositório oficial no navegador padrão do sistema operacional, facilitando o acesso ao código de maneira elegante.
// 5. *Atalhos do Teclado*: O menu superior "Playback" expõe as opções de controle e foca nos respectivos seletores de andamento e instrumentos ao serem selecionados, otimizando o fluxo de trabalho do usuário avançado.

= Descrição do Procedimento de Teste

A validação de corretude do sistema e do motor de áudio assíncrono abrangeu tanto testes unitários e arquivos de testes estruturados quanto testes de verificação manual do comportamento da interface e concorrência:

== Casos de Teste Automatizados

*Testes Unitários Automatizados*: Implementados no arquivo `tests/test_backend.cpp` com o framework `doctest`, estes testes rodam programaticamente e de forma isolada das dependências físicas do disco. Eles utilizam sequências textuais equivalentes definidas diretamente em memória para validar com precisão matemática os estados internos e o enfileiramento de eventos.

Os testes automatizados cobrem a corretude das seguintes regras e componentes do sistema:

- *Configurações e Limites das Vozes*: Garante que os atributos iniciais das vozes (soprano, contralto, tenor, baixo) correspondam aos padrões definidos e que o mapeamento de canais MIDI respeite a reserva do canal 9 (percussão), roteando as vozes corretas e ciclando adequadamente.
- *Mapeamento de Notas e Frequências*: Valida a conversão de caracteres de notas (incluindo alterações bemóis como `Eb`, `Ab`, `Mb`) para a escala de pitch MIDI, assegurando que oitavas fora do limite 0-9 sejam tratadas por meio de clamping.
- *Processamento Lexical e Eventos*: Valida a extração de eventos como notas de repouso (pausa), dinâmicas de volume (espaço duplo), modificações de oitavas (`?`, `.`, `V`), comandos de instrumentos (dígitos, vogais, caracteres `!`, `;`, `,`) e repetição de notas com consoantes.
- *Polifonia e Sincronismo*: Assegura que o parser lidará corretamente com múltiplas linhas e que a sintaxe de atraso `[n]` gera os tempos corretos de silêncio iniciais para cada voz.
- *Modificadores Globais*: Testa a aceleração e desaceleração de BPM (`>` e `<`), garantindo o despacho correto do evento de andamento global sincronizado.

== Verificação Manual de Concorrência e Estabilidade
*Casos de Teste Estruturados (arquivos `.txt`)*: O diretório `tests/` concentra arquivos de texto simples contendo composições de teste padronizadas (como `fuga_bpm.txt`, `fuga_oitavas.txt`, etc.). Esses arquivos são ignorados pela execução automatizada e servem como massa de teste estruturada para validação visual e auditiva manual, sendo carregados e reproduzidos diretamente através da interface gráfica (GUI).
- *Fuga BWV 847* e *Fuga BWV 578*: Validação do processamento polifônico complexo, garantindo que múltiplas vozes com atrasos de entrada `[n]` rodem concorrentemente sem falhas de sincronia.
- *Fuga BPM*: Testa o processamento dos modificadores globais de andamento `>` e `<`, garantindo que o BPM do sequenciador seja acelerado e desacelerado de forma homogênea para todas as trilhas ativas.
- *Fuga Instrumentos*: Valida se modificadores como dígitos e caracteres especiais `!`, `;`, `,`, vogais) resultam nas corretas trocas de canais de instrumentos do sintetizador FluidSynth.
- *Fuga Pausas*: Valida o comportamento de silenciamento temporário introduzido por caracteres minúsculos `a-h` e outras consoantes.
- *Fuga Oitavas*: Confirma que os comandos `?`, `V` e `.` alteram as oitavas locais nos limites definidos (0 a 9) e que o estouro superior reinicia à oitava de partida.
- *Fuga Repetidas*: Valida a semântica de repetição de nota ao encontrar consoantes ou caracteres não mapeados logo após uma nota válida.