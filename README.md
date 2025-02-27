# Jogo da Velha e Flappy Bird no BitDogLab com Raspberry Pi Pico W

Este projeto implementa dois jogos clássicos, Jogo da Velha e Flappy Bird, utilizando a ferramenta de desenvolvimento BitDogLab, que integra um Raspberry Pi Pico W e diversos periféricos como joystick ADC, botões físicos e uma matriz de LEDs 5x5 para exibição dos jogos.

## 📌 Requisitos
Este projeto foi desenvolvido para a BitDogLab, que já possui os seguintes periféricos integrados:
- Raspberry Pi Pico W
- Matriz de LEDs 5x5
- Joystick ADC
- Botões físicos (A, B e botão do joystick)
- Tela OLED SSD1306 (opcional para exibição de informações adicionais)

## 🎮 Jogos Implementados

### 🏁 Jogo da Velha
O jogo da velha é jogado na matriz de LEDs 5x5, onde os jogadores alternam suas jogadas usando o joystick para mover o cursor e o botão A para marcar a posição.

- O cursor se move nas posições válidas do tabuleiro.
- As jogadas são registradas e um vencedor é determinado automaticamente.
- O jogo pode ser reiniciado após um vencedor ser identificado ou em caso de empate.

### 🐦 Flappy Bird
Uma versão simplificada do Flappy Bird onde:
- O pássaro é representado por um LED na matriz.
- Os canos se movem da direita para a esquerda.
- O jogador pode pressionar um botão para fazer o pássaro subir.
- O jogo termina se o pássaro colidir com um cano ou sair da tela.
- Um sistema de pontuação é implementado.

## 🚀 Como Rodar o Projeto
1. Utilize a ferramenta BitDogLab com os periféricos já integrados.
2. Compile e carregue o código no Raspberry Pi Pico W usando o SDK do Raspberry Pi.
3. Após a inicialização, selecione o jogo desejado.
4. Jogue e divirta-se!

## 📜 Estrutura do Código
- **Inicialização dos periféricos:** Configuração dos pinos do joystick, botões e matriz de LEDs.
- **Lógica do Jogo da Velha:** Funções para atualização do cursor, verificação de vencedor e atualização da matriz de LEDs.
- **Lógica do Flappy Bird:** Atualização da posição do pássaro, movimentação dos canos e detecção de colisões.
- **Gerenciamento de entrada:** Leitura do joystick e botões para controle dos jogos.

## ✨ Melhorias Futuras
- Novos jogos para rodar na matriz de LEDs.

## 📌 Créditos
Desenvolvido para aprendizado e experimentação com a ferramenta de desenvolvimento BitDogLab e Raspberry Pi Pico W.

