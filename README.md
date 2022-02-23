# W5500
W5500 board Keystudio for home automation, simple sketchup for input/output with analog and digital pin

Ne pas oublier de changer le MAC, mqtt Broker, et NAME (lligne 7,10,11)


![Alt text](W5500.jpg?raw=true "Pinout")

W5500
** Output relay **
pins 4 = AR-test/ch/A/lgt/1/cmd/  1 = on 0 = off
pins 5 = AR-test/ch/A/lgt/2/cmd/  1 = on 0 = off
pins 6 = AR-test/ch/A/ht/1/cmd/  1 = on 0 = off
pins 7 = AR-test/ch/B/lgt/1/cmd/  1 = on 0 = off
pins 8 = AR-test/ch/B/lgt/2/cmd/  1 = on 0 = off
pins 9 = AR-test/ch/B/ht/1/cmd/  1 = on 0 = off
state_topic: "AR-01/ch/*/*/1/sta/" confirmation du changement de status

** Temperature DHT22 **
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

** Input Switch **
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