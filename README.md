# W5500
W5500 board Keystudio for home automation, simple sketchup for input/output with analog and digital pin

Ne pas oublier de changer le MAC, mqtt Broker, et NAME (lligne 7,10,11)


![Alt text](W5500.jpg?raw=true "Pinout")

W5500
## Output relay
- pins 4 = AR-test/ch/A/r/0/cmd/  1 = on 0 = off
- pins 5 = AR-test/ch/A/r/1/cmd/  1 = on 0 = off
- pins 6 = AR-test/ch/A/r/2/cmd/  1 = on 0 = off
- pins 7 = AR-test/ch/B/r/0/cmd/  1 = on 0 = off
- pins 8 = AR-test/ch/B/r/1/cmd/  1 = on 0 = off
- pins 9 = AR-test/ch/B/r/2/cmd/  1 = on 0 = off
- state_topic: "AR-01/ch/*/*/1/sta/" confirmation du changement de status

## Temperature DHT22

- pins A0 = AR-01/ch/A/cli/1/
- pins A1 = AR-01/ch/A/cli/2/
- pins A2 = AR-01/ch/B/cli/1/
- pins A3 = AR-01/ch/B/cli/2/

```
- platform: mqtt
name: "Alexis Chambre Master Temperature"
state_topic: "AR-01/ch/A/cli/1/"
unit_of_measurement: '°C'
value_template: "{{ value_json.temperature }}"
- platform: mqtt
name: "Alexis Chambre Master Humidity"
state_topic: "AR-01/ch/A/cli/1/"
unit_of_measurement: '%'
value_template: "{{ value_json.humidity }}"
- platform: mqtt
name: "Alexis Chambre Master Heat Index"
state_topic: "AR-01/ch/A/cli/1/"
unit_of_measurement: '°C'
value_template: "{{ value_json.heatindex}}"
```
## Input Switch 
- pins A4 = AR-01/ch/A/sw/1/s/
- pins A4 = AR-01/ch/A/sw/1/d/
- pins A4 = AR-01/ch/A/sw/1/l/
- pins A5 = AR-01/ch/A/sw/2/s/
- pins A5 = AR-01/ch/A/sw/2/d/
- pins A5 = AR-01/ch/A/sw/2/l/
- pins 2 = AR-01/ch/B/sw/1/s/
- pins 2 = AR-01/ch/B/sw/1/d/
- pins 2 = AR-01/ch/B/sw/1/l/
- pins 3 = AR-01/ch/B/sw/2/s/
- pins 3 = AR-01/ch/B/sw/2/d/
- pins 3 = AR-01/ch/B/sw/2/l/
- payload 1

```
- alias: Alexis Main Simple Click
  trigger:
    platform: mqtt
    topic: "AR-01/ch/A/sw/1/s/"
    # Optional
    payload: "1"
  action:
    - service: light.toggle
      target:
        entity_id: light.Alexislight
      data:
        brightness: 255
        kelvin: 2700

- alias: Alexis Main Double Click
  trigger:
    platform: mqtt
    topic: "AR-01/ch/A/sw/1/d/"
    # Optional
    payload: "1"
  action:
    - service: light.turn_on
      target:
        entity_id: light.Alexislight
      data_template:
        effect: "None"
        brightness: 50
        rgb_color: [255,0,0]

- alias: Alexis Main Long Click
  trigger:
    platform: mqtt
    topic: "AR-01/ch/A/sw/1/l/"
    # Optional
    payload: "1"
  action:
    - service: light.turn_on
      target:
        entity_id: light.Alexislight
      data:
        brightness: 255
        effect: "Random"
        # rgb_color:
        #   - "{{ range(255)|random }}"
        #   - "{{ range(255)|random }}"
        #   - "{{ range(255)|random }}"
```

FAQ:
- Board name,mac : 
  - AR-test = 0x00, 0xAA, 0xBB, 0xCC, 0xDD, 0xAA
  - AR-01 = 0x00, 0xAA, 0xBB, 0xCC, 0xDD, 0x

1- Pousser sur le bord, des fois les pins sont mal enfoncé.

