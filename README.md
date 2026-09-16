# WATER BILLING SYSTEM

## Overview

This is a simple, microcontroller based (Arduino) water billing system that aims to raise awareness on water usage by assigning users a limited amount of water litres in form of credits. Each user is given a RFID card which acts like the user's identity when using the system. The specific UID is read and matched with the specific user account when the card is scanned, and then the user's details (name, credit balance) are displayed on the LCD. The user may then interact with the system by the means of a keypad, through which the specific keys have specific functionalities, one being to allow the user to request a specific amount of water after the system prompts him/her to do so. If the litres required <= available credits in balance, the system allows water to be dipensed, deducting the taken litres from the balance. The flow rate is measured by a YF-S201 sensor that ensures the exact amount of litres are dispensed.

## Working mechanism (so far)

- A user is given a RFID card with a specific UID that represents the user's identity when interacting with the system.
- Once the person swipes the card on the RFID reader, the system does a search-and-match in the information stored in the program. If true, the user's details (age and credit balance) are displayed.
- Once displayed, the user will be prompted to enter the litres he/she wants to recieve by the means of a 4x3 keypad (using the keys `0-9`). If entered litres > present, then a notification will pop up on the LCD telling the user to recharge and no water will flow/be dispensed.
- If entered litres <= account balance, the relay valve will open and thus allowing the flow of water. Once the required amount is ejected, the solenoid valve closes due to the switching off of the relay, hence shutting the water flow.
    -**Flow measurement:** The YF-S201 generates pulses as water passes through the sensor, and the program counts these pulses using an interrupt on Arduino UNO pin D2: ```cpp
                                     attachInterrupt(digitalPinToInterrupt(FLOW_PIN), pulseCounter, FALLING);
                                     ```
    The current calibration factor being: ```cpp
                                          const float calibrationFactor = 450.0;
                                          ```
    The actual calibration factor may vary depending on various factors, so for accurate billing, the sensor should be calibrated experimentally before deployment.
- The amount of water dispensed is then deducted from the user's balance.
- Special keys have been implemented;
    **- the `*` key:- when pressed *once*, the entered litres is cleared (in case on has entered the wrong amount of litres), when pressed *twice* (within 700ms), an emergency stop is triggered; the system stop dispensing on the spot, "EMERGENCY STOP" is displayed, the current session is terminated, returning the user to the "Scan card..." stage.**
    **- the `#` key:- when pressed *once*, the entered amount is confirmed and the system proceeds with dispensing. When pressed *twice* (within 700ms), the credits of the users' are reset, and the user is returned to the litre-entry screen.** (The "double `#` press for the resetting of credits function is meant for the current prototype. Upon further development, this function will be removed.)
- So far, this has only been implemented for two accounts, i.e.: two cards;
    **- Joan Wambura - 10L credits**
    **- Prof. Nuhu - 20L credits**

### Safety features

The system includes a three-minute dispensing timeout: ```cpp
         if (millis() - startTime > 180000)
         ```
which prevents the valve from remaining open indefinitely if the expected flow measurement is not received.
The relay should also be connected appropriately for the voltage/current requirements of the solenoid valve.

### Example of the program in use

If a user has: ```text
               20 L
               ```

and enters: ```text
            5#
            ```

the system attempts to dispense: ```text
                                 5 L
                                 ```

After successful dispensing: ```text
                             Remaining: 15 L
                             ```

## Requirements and everything else needed to be done

### Hardware requirements (for the current prototype)

| Component | Quantity | Purpose |
| --- | ---: | --- |
| Arduino UNO | 1 | Main microcontroller |
| MFRC522 RFID Reader | 1 | Reads RFID card UIDs |
| RFID Cards/Tags | 2 (currently. More to be added in later versions) | User identification |
| 16×2 I2C LCD | 1 | Displays system/user information |
| 4×3 Matrix Keypad | 1 | User input |
| YF-S201 Flow Sensor | 1 | Measures water flow |
| 1-Channel Relay Module | 1 | Controls the water valve |
| Solenoid Water Valve | 1 | Controls physical water flow |
| Appropriate power supply | 1 | Powers the system/valve |
| Jumper wires | As required | Connections |
| Water tubing/plumbing | As required | Water path |

**NB:** This code was written for an Arduino *UNO*.

### Software requirements

An **Arduino IDE 2.x (2.3.x or newer)** is recommended for this project.
    - The following Arduino libraries are required:

    | Library | Purpose |
    | --- | --- |
    | `SPI` | Communication between the Arduino and MFRC522 RFID reader |
    | `MFRC522` | RFID card detection and UID reading |
    | `Wire` | I2C communication |
    | `LiquidCrystal_I2C` | Controls the I2C LCD |
    | `Keypad` | Controls the 4×3 matrix keypad |
        - of which `SPI` and `Wire` are built-in libraries which are normally included with the Arduino IDE and do not normally need to be installed separately. The others can be installed through **Arduino IDE → Library Manager**; where one has to search for and install: ```text
                 MFRC522
                 LiquidCrystal I2C
                 Keypad
                 ```
        as this code uses: ```cpp
                           #include <SPI.h>
                           #include <MFRC522.h>
                           #include <Wire.h>
                           #include <LiquidCrystal_I2C.h>
                           #include <Keypad.h>
                           ```
    - Choosing a board: In the Arduino IDE, select: **Tools → Board → Arduino AVR Boards → Arduino Uno**, then select the COM port corresponding to the connected Arduino, i.e.: **Tools → Port → [Arduino COM Port]**. The exact COM port will depend on the computer.

### Wiring

1. ***MFRC522 RFID Reader***

    | MFRC522 | Arduino UNO |
    | --- | --- |
    | SDA / SS | D10 |
    | SCK | D13 |
    | MOSI | D11 |
    | MISO | D12 |
    | RST | D9 |
    | 3.3V | 3.3V |
    | GND | GND |

    **Important:** The MFRC522 module is a **3.3V device**. Do not power its VCC from the Arduino UNO's 5V pin.

2. ***16×2 I2C LCD***

    | LCD | Arduino UNO |
    | --- | --- |
    | VCC | 5V |
    | GND | GND |
    | SDA | A4 |
    | SCL | A5 |

    The code assumes the LCD I2C address is: ```text
                                             0x27
                                             ```

    So if the LCD does not respond, its address may be different and may need to be changed in the code.

3. ***4×3 Keypad***

    | Keypad | Arduino UNO |
    | --- | --- |
    | Row 1 | A0 |
    | Row 2 | A1 |
    | Row 3 | A2 |
    | Row 4 | A3 |
    | Column 1 | D3 |
    | Column 2 | D4 |
    | Column 3 | D5 |

4. ***YF-S201 Flow Sensor***

    | YF-S201 | Arduino UNO |
    | --- | --- |
    | Signal | D2 |
    | VCC | 5V* |
    | GND | GND |

    \*Follow the voltage requirements of the specific YF-S201 module being used.

    The signal is connected to **D2**, which is used as an external interrupt on the Arduino UNO.

5. ***Relay Module***

    | Relay | Arduino UNO |
    | --- | --- |
    | IN | D7 |
    | VCC | 5V |
    | GND | GND |

    The relay is used to control the solenoid valve.

    **Do not connect the solenoid valve directly to an Arduino GPIO pin.**

    The valve should have an appropriate external power supply, with the relay acting as the switching device.

#### Pin Summary

| Arduino Pin | Component | Function |
| --- | --- | --- |
| D2 | YF-S201 | Flow sensor pulse input |
| D3 | Keypad | Column 1 |
| D4 | Keypad | Column 2 |
| D5 | Keypad | Column 3 |
| D7 | Relay | Valve control |
| D9 | MFRC522 | RFID reset |
| D10 | MFRC522 | RFID chip select |
| D11 | MFRC522 | SPI MOSI |
| D12 | MFRC522 | SPI MISO |
| D13 | MFRC522 | SPI clock |
| A0 | Keypad | Row 1 |
| A1 | Keypad | Row 2 |
| A2 | Keypad | Row 3 |
| A3 | Keypad | Row 4 |
| A4 | LCD | I2C SDA |
| A5 | LCD | I2C SCL |

***NB: Registered users so far (in this prototype version)***

| User | RFID UID | Initial Credits |
| --- | --- | ---: |
| JOAN WAMBURA | `230835AA` | 10 L |
| Prof. NUHU | `236D7F11` | 20 L |

To add another user, add their UID, name, and starting credits to the source code and include the UID in the authentication logic.

### Tip: Finding an RFID's card UID (when registering a new one)

Before registering a new card, use an MFRC522 UID-reading sketch to scan the card. The Serial Monitor will display something similar to: ```text
                                           Card UID: A3 B1 6F 94
                                           ```
The UID must then be converted into the format used by the program, i.e.: ```text
                      A3 B1 6F 94
                      ```
becomes: ```cpp
         const String USER_UID = "A3B16F94";
         ```

### Uploading the program

1. Install Arduino IDE 2.x.
2. Install the required libraries.
3. Connect the Arduino UNO using USB.
4. Open the `.ino` file.
5. Select **Arduino UNO** under **Tools → Board**.
6. Select the correct COM port under **Tools → Port**.
7. Click **Verify** to compile the program.
8. If compilation succeeds, click **Upload**.
9. Open **Serial Monitor** at: ```text
                               9600 baud
                               ```
The Serial Monitor should display: ```text
                                   System Ready
                                   ```
The LCD should display: ```text
                        Water Billing
                        Scan Card...
                        ```

### Using the system

1. Power the system.
2. Wait for the LCD to display `Scan Card...`.
3. Scan a registered RFID card.
4. The user's name and available credits are displayed.
5. Enter the desired amount of water in litres using the keypad.
6. Press `#` to confirm.
7. The relay activates the water valve.
8. The YF-S201 measures the water being dispensed.
9. When the requested amount is reached, the relay switches off.
10. The consumed amount is deducted from the user's credits.
11. The user may request additional water while the session remains active.

### Troubleshooting

- **RFID does not detect cards**: Check if -MFRC522 is powered from 3.3V.
                                           - SDA is connected to D10.
                                           - RST is connected to D9.
                                           - SPI connections are correct.
                                           - `MFRC522` library is installed.
                                           - The card is within the reader's operating range.
- **LCD is blank**: Check -VCC and GND.
                          - SDA → A4.
                          - SCL → A5.
                          - LCD I2C address.
As this code currently uses: ```cpp
                             LiquidCrystal_I2C lcd(0x27, 16, 2);
                             ```
and some LCD modules use another address such as `0x3F`.
- **Keypad doesn't work**: Check that the row and column wires match, i.e.: ```cpp
                          byte rowPins[ROWS] = {A0, A1, A2, A3};
                          byte colPins[COLS] = {3, 4, 5};
                          ```
- **Incorrect flow measurement:** The YF-S201 calibration factor may need adjustment, i.e.: ```cpp
                                              const float calibrationFactor = 450.0;
                                              ```
Measure a known volume of water and adjust the factor accordingly.
- **Relay behaves incorrectly**: Check the relay module's trigger logic. Some relay modules are *active LOW*, while this program currently assumes an *active HIGH* control signal, i.e.: ```cpp
                                    digitalWrite(RELAY_PIN, HIGH);
                                    ```
If your relay is active LOW, the relay logic will need to be inverted.

## Anticipated

- Support for multiple users: increase the system's flexibility to support more people.
- A registration function that allows new users to be added to the system and associated with their respective RFID cards.
- Creation of an offline app that will act as a credit recharge system. Users will be able to request or purchase additional credits from the supplier. The application will be integrated with the system.
- The creation/presence of the "supplier's side" for suppliers' to create their accounts and manage users as well as replenish their credits through them within that offline app.
- Usage records: Store each user's water consumption history, including the amount of water used and remaining credits and displaying it to the user when prompted.
- Persistent user accounts: Store user information and credit balances in non-volatile memory or an external database (Maybe even in a cloud) so that information is not lost when the Arduino is restarted.
- Monitoring and reporting: Provide the supplier with information about water consumption, remaining credits, and overall system usage.
- Automatic leak detection.
- More sophisticated authentication.

## License

This project is currently to be used for educational purposes and is still a prototype, so the user credits are currently defined in the program itself and are reset when the Arduino restarts or when a new RFID session is created.

## Project Goals

My aim is to develop the prototype into a full practical water management and billing system that encourages responsible water consumption and usage while providing a simpler user interface for the monitoring and management of water usage.
