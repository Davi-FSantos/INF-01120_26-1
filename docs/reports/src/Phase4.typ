#import "@preview/touying:0.7.4": *
#import themes.metropolis: *

#import "@preview/numbly:0.1.0": numbly

#import "@preview/icu-datetime:0.1.2": fmt-date

/* The power of typst */
#set text(lang: "pt", region: "br")

#set text(font: "Liberation Sans", size: 20pt)

#show: metropolis-theme.with(
  aspect-ratio: "16-9",
  // align: horizon,
  config-common(
//     handout: true,
//     show-notes-on-second-screen: right
  ),
  config-colors(
    primary: rgb(255, 0, 0),
    secondary: rgb(0, 0, 0,),
    neutral-darkest: rgb(0, 0, 0),
    neutral-lightest: rgb(255, 255, 255),
  ),
  config-info(
    title: "Music Machine",
    subtitle: "INF-01120 - Desenvolvimento de Software",
    author: "Davi F Santos, Gabriel Schons, Gabriel Copatti, Julio Augusto de Castilhos Borges \n",
    date: fmt-date(datetime.today(), locale: "pt", length: "long"),
    logo: image("images/inf-logo-white.svg", width: 3cm),
  ),
)

#set heading(numbering: numbly("{1}.", default: "1.1"))

#title-slide()


= Build System & Ferramentas

== O Problema das Plataformas <touying:hidden>

A equipe desenvolve o projeto em diferentes ambientes e sistemas operacionais:

- *Davi*: macOS e Linux
- *Julio*: Linux
- *Schons*: Linux
- *Copatti*: Windows

#pause
#v(1em)
*Objetivo:* Precisamos de um Build System robusto para suportar todas as plataformas dos desenvolvedores de forma unificada.

== A Solução: Xmake <touying:hidden>

#alternatives[
  - Buscamos uma ferramenta multiplataforma (Linux, Windows e macOS) ágil e de fácil configuração.
][
  - Buscamos uma ferramenta multiplataforma (Linux, Windows e macOS) ágil e de fácil configuração.
  
  - *Resposta:* Xmake
]

#pause
#v(1em)
*Por que escolhemos o Xmake?*
- Configuração baseada em Lua, garantindo scripts simples e legíveis.
- Gerenciamento nativo e integrado de dependências.
- Excelente suporte multiplataforma para C/C++.
- Tempos de compilação altamente otimizados.
- Package Manager *incluso* para bibliotecas e outros. 


= Linter e Qualidade de Código

== Escolha do Linter <touying:hidden>

Para garantir a padronização e a integridade do código fonte, adotamos as ferramentas do ecossistema LLVM:

#pause
#v(0.5em)
- *clang-format*
  - Padronização automática do estilo de código.
  - Mantém consistência de indentação, espaçamento e quebras de linha entre todos os desenvolvedores.

#pause
#v(0.5em)
- *clang-tidy*
  - Análise estática avançada (linter).
  - Detecta bugs em potencial, sugere melhorias de performance e garante o uso de boas práticas modernas em C++.








= Classes e UML

== Classes <touying:hidden>

  - *Voice*: Encapsula os parâmetros (instrumento, volume, oitava, delay) e a fila de eventos musicais de uma trilha.
  - *ITextParser / TextParser*: Analisa as linhas de texto, processa a sintaxe `[n]` e enfileira as notas e pausas nas vozes.
  - *MidiPlayer*: Controla o loop de reprodução em thread dedicada, enviando os eventos de notas em tempo real e gerenciando o BPM.
  - *AudioEngine*: Abstração física do sintetizador FluidSynth, gerenciando canais MIDI, timbres e controle de volume físico.
  - *MidiWriter*: Responsável por serializar a fila de eventos de todas as vozes em trilhas independentes de um arquivo `.mid`.

== UML <touying:hidden>

  #image("images/UML_16_9.svg")

// #components.adaptive-columns(outline(title: none, indent: 1em))

= Interface Gráfica

== Biblioteca <touying:hidden>

#alternatives[
 - Precisamos de uma biblioteca multiplataforma, com bindings para C++
][
  #align(center + horizon)[
    #grid(
      columns: (auto, auto),
      gutter: 2em,
      align: horizon,
      [Assim, escolhemos #box(baseline: 25%, image("images/Qt-logo-neon.webp", width: 1.5em))],
      box(width: 16cm, height: 100%)[
        #place(top + center, dx: 0pt, dy: 0pt, image("images/macos.png", width: 15cm))
        #place(top + center, dx: 10%, dy: 24%, image("images/linux.png", width: 13.08cm))
        #place(top + center, dx: 2*10%, dy: 1.55*24%, image("images/windows.png", width: 13.08cm))
      ]
    )
  ]
][
  #align(center + horizon)[
    #grid(
      columns: (auto, auto),
      gutter: 2em,
      align: horizon,
      [Assim, escolhemos #box(baseline: 25%, image("images/Qt-logo-neon.webp", width: 1.5em))],
      box(width: 16cm, height: 100%)[
        #place(top + center, dx: 0pt, dy: 0pt, image("images/originalmacos.png", width: 15cm))
        #place(top + center, dx: 10%, dy: 24%, image("images/lingrayscale.png", width: 13.08cm))
        #place(top + center, dx: 2*10%, dy: 1.55*24%, image("images/originalwindows.png", width: 13.08cm))
      ]
    )
  ]
]  
    



== Menubar <touying:hidden>

#slide(self => [
  #let (uncover, only, alternatives) = utils.methods(self)
#align(center)[
  #box(height: 9cm)[
    #align(top)[
      #alternatives[
        #image("images/menubar.png", width: 80%)
      ][
        #image("images/file.png", width: 80%)
      ][
        #image("images/fileexport.png", width: 80%)
      ][
        #image("images/edit.png", width: 80%)
      ][
        #image("images/playback.png", width: 80%)
      ][
        #image("images/help.png", width: 80%)
      ]
    ]
  ]
]

#meanwhile

- *Menubar*: #pause Contendo menu de arquivos#pause#pause, edição de texto, #pause controles do som #pause e ajuda.
])

== Toolbar <touying:hidden>

- *Toolbar*: Contendo Botão para abrir #pause e salvar arquivos, #pause mudar o volume#pause, iniciar #pause e resetar a 'máquina musical'#pause, além de mudar o instrumento #pause e por fim, mudar o BPM.

#meanwhile

#align(center)[
  #image("images/mainWindow.png", width: 80%)
]

== Entrada de Texto <touying:hidden>

#align(center)[
  #image("images/mainWindow.png", width: 80%)
]

= Demonstração <touying:hidden>

#align(center)[
  #link("src/launch.sh")[Launch Music Machine]
]



= Problemas Não Resolvidos

== Desafios em Aberto <touying:hidden>

- *Bancos de Timbres (SoundFonts)*
  - Implementação e suporte adequado aos formatos `.sf2` e `.sf3`.

#pause
#v(0.5em)
- *Gerenciamento de Arquivos Pesados*
  - Prós e contras de incluir um arquivo `.sf2` de tamanho elevado diretamente no controle de versão.

#pause
#v(0.5em)
- *Estratégia para resolver o Soundfont*
  - Adicionar uma nova dependência de rede (`curl`) para efetuar o download automático do `.sf2` durante a compilação.
  - _versus_
  - Manter uma compilação totalmente estática sem dependências de rede externas.





== Obrigado <touying:hidden>

#place(center + horizon, image("images/inf-logo-white.svg", width: 5cm))


/*
  Ordem de Apresentação:
  Todos Oizinho -> Davi -> Schons/Copatti -> Julio -> Davi -> Todos Tchauzinho
                -> Build System -> Classes -> UI -> Não atingidos
*/

