# 📡 IoT Vinheria Agnelo – ESP32 + FIWARE

Este projeto implementa uma solução **IoT** utilizando **ESP32**, sensores de ambiente e integração com a arquitetura **FIWARE**.

Ele faz a leitura de:

* 🌡️ **Temperatura e Umidade** (sensor **DHT22**)
* 💡 **Luminosidade** (sensor **LDR**)
* 🖥️ Exibe informações em um **LCD I2C (16x2)**

Os dados coletados são enviados via **MQTT** para um **IoT Agent**, processados pelo **Orion Context Broker** e podem ser acessados ou integrados com outros sistemas.

---

## 📂 Estrutura do Projeto

```
.
├── src/
│   └── main.cpp        # Código principal do ESP32
├── docker-compose.yml  # Infraestrutura FIWARE (MQTT, Orion, IoT Agent, STH-Comet)
├── postman_collection/ # Collection para configurar e testar o ambiente
└── README.md
```

---

## ⚙️ Tecnologias Utilizadas

* **ESP32 DevKit** + **PlatformIO** (VSCode)
* **Wokwi** (simulação do hardware)
* **FIWARE Components**:

  * **Mosquitto** (broker MQTT)
  * **IoT Agent**
  * **Orion Context Broker**
  * **STH-Comet** (persistência de históricos)
* **Arduino Libraries**:

  * `WiFi.h`
  * `PubSubClient.h`
  * `ArduinoJson.h` (v7)
  * `DHT.h`
  * `LiquidCrystal_I2C.h`

---

## 🛠️ Como Rodar

### 0. Simulação no Wowki

* Link para o Wokwi: [Projeto](https://wokwi.com/projects/443110789461533697)

### 1. Simulação no Wokwi + PlatformIO

* Abra o projeto no **VSCode** com a extensão **PlatformIO** instalada.
* Configure a simulação no **Wokwi** com ESP32 + DHT22 + LDR + LCD I2C.
* Compile e rode o projeto.

> O ESP32 conecta ao Wi-Fi (rede Wokwi), lê os sensores e envia os dados via MQTT.

---

### 2. Subindo a Infra no Docker

Na raiz do projeto:

```bash
docker-compose up -d
```

Serviços disponíveis:

* Mosquitto MQTT → `localhost:1883`
* Orion Context Broker → `localhost:1026`
* IoT Agent → `localhost:4041`
* STH-Comet → `localhost:8666`

---

### 3. Configuração no Postman

* Importe a collection disponível em `postman_collection/`.
* Configure os dispositivos no IoT Agent.
* Teste a publicação e leitura dos dados no Orion.

---

## Comandos Executados na Maquina Ubunto

```bash
sudo su
```

```bash
apt update
```

```bash
apt-get update
```

```bash
apt install docker.io
```

```bash
apt install docker-compose
```

```bash
exit
```

```bash
git clone https://github.com/TechVerseFiap/CP04.VinheriaAgnelo.Edge.git
```

```bash
cd CP04.VinheriaAgnelo.Edge
```

```bash
sudo docker-compose up -d
```

```bash
sudo docker ps
```


## 📡 Fluxo de Dados

1. **ESP32** coleta dados (luminosidade, umidade, temperatura).
2. Dados são enviados como JSON via **MQTT**:

   ```json
   {
     "luminosity": 45,
     "humidity": 55,
     "temperature": 22.3
   }
   ```
3. **IoT Agent** recebe e transforma em entidades FIWARE.
4. **Orion Context Broker** gerencia o contexto.
5. **STH-Comet** persiste séries temporais.
6. Dados podem ser consultados ou integrados com outros serviços.

---

## 📸 Exemplo de Saída no LCD

* “Ambiente muito CLARO”
* “Umidade ALTA: 80%”
* “Temp. OK: 22°C”

---

## 👤 Autores

- Lucas Perez Bonato - 565356
- Lucas dos Reis Aquino - 562414
- Eduarda de Castro Coutinho dos Santos - 562184
- Diogo Oliveira Lima - 562559
- Leandro Simoneli da Silva - 566539

