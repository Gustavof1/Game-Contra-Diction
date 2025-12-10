# Contra-Diction

## Gameplay
https://youtu.be/BA2nu5bin9o

[![Gameplay](Gameplay%20Images/contra-diction.jpg)](https://youtu.be/BA2nu5bin9o)

## Screenshots

### Inventário
![Inventory](Gameplay%20Images/Inventory.png)

### Debug
![Debug](Gameplay%20Images/DebugImage.png)

## Descrição

Contra-Diction é um jogo de ação e plataforma 2D desenvolvido em C++ utilizando a biblioteca SDL2 e OpenGL. O jogo coloca o jogador no papel de um astronauta perdido em um planeta diferenciado, deve explorar cenários não usuais repletos de alienígenas e robôs. A jogabilidade combina elementos clássicos de "run and gun" com "trolagem".

A narrativa do jogo envolve um astronauta distraído que acabou caindo em um planeta separado de sua nave e precisa encontrar uma forma de pedir ajuda, inicialmente ele precisa encontrar destroços da nave para enviar um sinal para o espaço, mas no caminho ele irá encontrar obstáculos. O objetivo principal é atravessar os níveis, enfrentando armadilhas, ameaças e escolhas. O jogador deve aprender os obstaculos conforme ele tenta repetidamente os níveis e perceber que as suas escolhas possuem impacto no que acontece no jogo. O jogador possui de um arsenal que inclui lasers e projéteis, além de habilidades de movimentação como pulo e agachamento para desviar de ataques.

O jogo apresenta um sistema de inventário para gerenciar itens coletados, inicialmente os itens estão todos desbloqueados para fins de teste. A vitória é alcançada ao completar todas as fases e derrotar o chefe final. A derrota ocorre caso o personagem tome um hit. Pode parecer difícil, mas com prática e persistência é possível avançar no jogo sem problemas.

Ferramentas usadas:

Tilesets, sprites e sons de https://itch.io/game-assets/
Spritesheets e animações de personagens criados no https://www.makko.ai/ (novembro/dezembro 2025)
Imagens de fundo e cutscenes https://gemini.google.com/


## Funcionalidades a serem testadas

Durante o playtesting, por favor, observem com atenção as seguintes funcionalidades:

*   **Movimentação do Personagem:** Verifique a fluidez ao andar, pular e agachar. Teste as colisões com o cenário e plataformas.
*   **Combate:** Teste o sistema de tiro, a precisão dos projéteis e o efeito causado aos inimigos. Verifique se os inimigos reagem corretamente e se a detecção de acertos está precisa.
*   **Inteligência Artificial:** Observe o comportamento dos inimigos (perseguição, ataque, patrulha).
*   **Interface e Menus:** Navegue pelos menus (Principal, Pausa, Inventário) e verifique se todas as opções funcionam como esperado e se o visual está muito confuso.
*   **Transição de Fases:** Certifique-se de que a passagem de um nível para o outro ocorre sem erros.
*   **Inimigos:** Teste a batalha contra os inimigos e armadilhas, verificando seus padrões de ataque, dificuldade e visibilidade.
*   **Performance:** Observe se há quedas de taxa de quadros ou travamentos durante o jogo. (Ao escolher um estágio o jogo pode travar pois está carregando, espere alguns segundos)

## Créditos

**Desenvolvedores:**

*   Gustavo Ferreira  - Programação, Arte, Som, Game Design, Menus, Cutscene, Narrativa, Inimigos
*   Mateus Silva - Programação, Game Design, Criação de Mapas, Planejamento, Inimigos
*   Vinicius Trindade - Programação, Planejamento, Som, Menus, Engine de Base