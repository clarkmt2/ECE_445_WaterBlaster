/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

#include "newhaven_slim_oled.h"

    int use_sensors = 0;
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

int read_pressure_psi(void) {
    HAL_ADC_Start(&hadc1);

    if (HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY) != HAL_OK) {
        return -1;
    }

    uint32_t adc_raw_value = HAL_ADC_GetValue(&hadc1);
    float voltage = ((adc_raw_value * 3.3f) / 4095.0f) * 2.0f;

    if (voltage < 0.7f) voltage = 0.7f;
    if (voltage > 4.5f) voltage = 4.5f;

    float pressure = ((voltage - 0.7f) / 3.8f) * 150.0f;

    // Proper rounding for both positive and negative values
    if (pressure >= 0.0f)
        return (int)(pressure + 0.5f);
    else
        return (int)(pressure - 0.5f);
}
/* USER CODE BEGIN PV */
static uint8_t last_encoded = 0;

int read_encoder_delta(void) {
    static uint8_t last_state = 0;
    static int8_t delta_accumulator = 0;

    uint8_t a = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_2);
    uint8_t b = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_3);

    uint8_t new_state = (a << 1) | b;

    // Only act if state actually changed
    if (new_state != last_state) {
        // Combine old and new state into 4-bit transition code
        uint8_t transition = (last_state << 2) | new_state;
        last_state = new_state;

        // Decode direction
        switch (transition) {
            case 0b0001:
            case 0b0111:
            case 0b1110:
            case 0b1000:
                delta_accumulator++;
                break;

            case 0b0010:
            case 0b1011:
            case 0b1101:
            case 0b0100:
                delta_accumulator--;
                break;

            default:
                // Illegal transition or bounce; ignore
                break;
        }

        // Return ±1 only when 4 steps in one direction accumulated
        if (delta_accumulator >= 4) {
            delta_accumulator = 0;
            return 1;
        } else if (delta_accumulator <= -4) {
            delta_accumulator = 0;
            return -1;
        }
    }

    return 0;
}


char OLED[4][20];
char Buffer[4][20]; // staging buffer to write to

void clear_buffer(void) {
    for (int row = 0; row < 4; row++) {
        memset(Buffer[row], ' ', 20);
    }
}




void update_display_chunks(char Buffer[4][20]) {
    for (uint8_t row = 0; row < 4; row++) {
        uint32_t *oled_row = (uint32_t *)OLED[row];
        uint32_t *buf_row  = (uint32_t *)Buffer[row];

        for (uint8_t chunk = 0; chunk < 5; chunk++) { // 20 chars / 4 = 5 chunks
            if (oled_row[chunk] != buf_row[chunk]) {
                // There's a difference in this chunk — do per-char check
                uint8_t base_col = chunk * 4;

                for (uint8_t offset = 0; offset < 4; offset++) {
                    uint8_t col = base_col + offset;
                    if (Buffer[row][col] != OLED[row][col]) {
                        NHD_OLED_cursorPos(row, col);
                        NHD_OLED_sendData(Buffer[row][col]);
                        OLED[row][col] = Buffer[row][col];
//                        Buffer[row][col] = 0x20;
                    }
                }
            }
        }
    }
}


void pad_center(char dst[20], const char *src) {
    int len = strlen(src);
    if (len > 20) len = 20;

    int pad_left = (20 - len) / 2;
    int pad_right = 20 - len - pad_left;

    memset(dst, ' ', 20);
    memcpy(dst + pad_left, src, len);
}

void pad_right_20(char dst[20], const char *src) {
    int len = strlen(src);
    if (len > 20) len = 20;

    memset(dst, ' ', 20);
    memcpy(dst + (20 - len), src, len);
}

void pad_left_20(char dst[20], const char *src) {
    int len = strlen(src);
    if (len > 20) len = 20;

    memcpy(dst, src, len);
    if (len < 20)
        memset(dst + len, ' ', 20 - len);
}

void enter_emergency_shutdown(void) {
    // Immediately stop everything
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET); // valve closed
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_RESET); // pump off
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_RESET); // LED1 off
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_RESET); // LED2 off
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET); // LED3 off

    clear_buffer();
    pad_center(Buffer[1], "!! LEAK DETECTED !!");
    pad_center(Buffer[2], "Shutdown.");
    update_display_chunks(Buffer);

    uint32_t shutdown_start = HAL_GetTick();

    while (HAL_GetTick() - shutdown_start < 5000) {
        HAL_Delay(10); // Wait quietly for 5 seconds
    }

    NHD_OLED_displayOff(); // Turn off OLED after 5 sec

    // Now sit here doing nothing until a reset button (e.g., PB3) is pressed
    while (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_6) == GPIO_PIN_SET) {
        // Waiting for PB3 to be pressed
        HAL_Delay(10);
    }

    // Optionally: Hard reset microcontroller when button is pressed
    NVIC_SystemReset();
}

void monitor_status(void) {
	int current_psi = read_pressure_psi();
	if(use_sensors == 1){
		if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_3) == GPIO_PIN_RESET || HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_4) == GPIO_PIN_RESET || HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_15) == GPIO_PIN_RESET || current_psi > 125){


		// Immediately stop everything
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET); // valve closed
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_RESET); // pump off
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_RESET); // LED1 off
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_RESET); // LED2 off
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET); // LED3 off

		clear_buffer();
		pad_center(Buffer[1], "!! LEAK DETECTED !!");
		pad_center(Buffer[2], "Shutdown.");
		update_display_chunks(Buffer);

		uint32_t shutdown_start = HAL_GetTick();

		while (HAL_GetTick() - shutdown_start < 5000) {
			HAL_Delay(10); // Wait quietly for 5 seconds
		}

		NHD_OLED_displayOff(); // Turn off OLED after 5 sec

		// Now sit here doing nothing until a reset button (e.g., PB3) is pressed
		while (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_7) == GPIO_PIN_SET) {
			// Waiting for PB3 to be pressed
			HAL_Delay(10);
		}

		// Optionally: Hard reset microcontroller when button is pressed
		NVIC_SystemReset();
		}
	}
}
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
// Declare a global variable to track the last tick
/*
 *
 *
 0 = home page
 1 = settings for shot length
 *
 */
/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
    uint32_t previousTick = 0;
    uint32_t lastPressureUpdate = 0;
    static int last_pressure_value = -1;

    int shots = 0;
    int page = 0;
    int shot_length = 1000;
    int shot_max = 10000;
    int shot_min = 20;
    int pressed = 0;
    int fire_type = 0;
    int ignore_shots_remaining = 1;
    char message[20];
    static uint8_t refill_button_was_down = 0;
    uint8_t length;
	static uint8_t emergency_previous = 1;
	const char *settings_options[] = {
	    "Shot Length",
	    "Firing Mode",
	    "Drain Tank",
	    "Safety Monitoring",
		"Fire Bypass"
	};
	const int settings_count = sizeof(settings_options) / sizeof(settings_options[0]);

	int settings_selection = 0;   // Which item is selected (0 to 5)
	int settings_scroll = 0;      // Topmost visible option index


    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_ADC1_Init();
    HAL_ADC_Start(&hadc1);

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET);
    HAL_Delay(500);
    NHD_OLED_begin();
    NHD_OLED_textClear();

    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET); // Valve CLOSED on startup
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET);

    // Force PB5 to be input with no pull
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    page = 100;
    int power_pressed = 0;

    while (1) {
        uint8_t button_now = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_7);

        if (button_now == GPIO_PIN_RESET) { // Button pressed
            if (power_pressed == 0) { // Only act if wasn't already pressed

                HAL_Delay(10); // 🔥 Add debounce delay (10ms typical)

                if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_7) == GPIO_PIN_RESET) {
                    // Still pressed after delay ➔ valid press
                    if (page == 100) {
                        page = 0; // Turn display ON
                    } else {
                        page = 100; // Turn display OFF
                        clear_buffer();
                        update_display_chunks(Buffer);
                    }
                }
            }
            power_pressed = 1;
        } else {
            power_pressed = 0;
        	if(page == 100) continue;

			monitor_status();


			if (!HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_5)) {
				if (pressed != 1) {
					page = (page == 0) ? 1 : 0;
					clear_buffer();
					update_display_chunks(Buffer);
					pressed = 1;
				}
			} else {
				pressed = 0;
			}

			if (page == 0) {
				pad_center(Buffer[0], "Home Page");

				if (HAL_GetTick() - lastPressureUpdate >= 250) {
					last_pressure_value = read_pressure_psi();
					lastPressureUpdate = HAL_GetTick();
				}

				snprintf(message, sizeof(message), "%d PSI", last_pressure_value);
				pad_center(Buffer[1], message);

				snprintf(message, sizeof(message), "Shots Remaining: %d", shots);
				pad_center(Buffer[2], message);

				// trigger
				if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_11) == GPIO_PIN_RESET) {
				    switch (fire_type) {
				        case 0: // Single Shot
				            if (shots > 0 || ignore_shots_remaining) {
				                HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_9);
				                HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET); // valve open
				                pad_center(Buffer[3], "Firing");
				                update_display_chunks(Buffer);
				                HAL_Delay(shot_length);
				                HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET); // valve closed
				                HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_9);
				                pad_center(Buffer[3], "Shot Fired");
				                update_display_chunks(Buffer);
				                shots--;
				                if (shots < 0) shots = 0;
				            } else {
				                pad_center(Buffer[3], "Out of Shots!");
				                update_display_chunks(Buffer);
				                HAL_Delay(1500);
				                pad_center(Buffer[3], " ");
				            }
				            break;

				        case 1: // Burst Fire (3 shots)
				            for (int i = 0; i < 3; i++) {
				                if (shots <= 0 && !ignore_shots_remaining) break;
				                HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_9);
				                HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET); // valve open
				                pad_center(Buffer[3], "Burst Shot");
				                update_display_chunks(Buffer);
				                HAL_Delay(shot_length);
				                HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET); // valve closed
				                HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_9);
				                shots--;
				                if (shots < 0) shots = 0;
				                HAL_Delay(100); // delay between burst shots
				            }
				            pad_center(Buffer[3], "Burst Done");
				            update_display_chunks(Buffer);
				            break;

				        case 2: // Stream (hold until trigger released)
				            pad_center(Buffer[3], "Streaming...");
				            update_display_chunks(Buffer);

				            HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_9);
				            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET); // valve open

				            while (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_11) == GPIO_PIN_RESET) {
				                if (shots <= 0 && !ignore_shots_remaining) break;
				                HAL_Delay(10); // short loop delay
				                // optionally update shots/time/pressure live here
				            }

				            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET); // valve closed
				            HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_9);

				            pad_center(Buffer[3], "Stream Ended");
				            update_display_chunks(Buffer);

				            if (!ignore_shots_remaining) {
				                // Estimate how many shots to deduct based on time
				                // (Optional improvement: measure real time instead of guessing)
				                shots--;
				                if (shots < 0) shots = 0;
				            }
				            break;
				    }
				}


				uint8_t refill_button = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_6);
				if (refill_button == GPIO_PIN_RESET && !refill_button_was_down) {
					refill_button_was_down = 1;

					int cancelled = 0;
					int refill_button_was_released = 0;
					int current_psi = read_pressure_psi();

					HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET); // open valve
					HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_SET);   // pump on
					HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET);   // LED on

					pad_center(Buffer[3], "Refill: Valve Open");
					snprintf(message, sizeof(message), "%d PSI", current_psi);
					pad_center(Buffer[1], message);
					update_display_chunks(Buffer);

					while (1) {
						monitor_status();

						if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_6) == GPIO_PIN_SET) refill_button_was_released = 1;
						if (refill_button_was_released && HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_6) == GPIO_PIN_RESET) {
							cancelled = 1;
							break;
						}
						current_psi = read_pressure_psi();
						pad_center(Buffer[3], "Refill: Valve Open");
						snprintf(message, sizeof(message), "%d PSI", current_psi);
						pad_center(Buffer[1], message);
						update_display_chunks(Buffer);
						if (current_psi >= 12) {
							HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET); // close valve immediately
							pad_center(Buffer[3], "Refill: Valve Shut");
							snprintf(message, sizeof(message), "%d PSI", current_psi);
							pad_center(Buffer[1], message);
							update_display_chunks(Buffer);
							break;
						}
						HAL_Delay(100);
					}

					while (!cancelled && current_psi < 70) {
						monitor_status();
						if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_6) == GPIO_PIN_SET) refill_button_was_released = 1;
						if (refill_button_was_released && HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_6) == GPIO_PIN_RESET) {
							cancelled = 1;
							break;
						}
						current_psi = read_pressure_psi();
						pad_center(Buffer[3], "Refill: Charging");
						snprintf(message, sizeof(message), "%d PSI", current_psi);
						pad_center(Buffer[1], message);
						update_display_chunks(Buffer);
						HAL_Delay(100);
					}

					if (!cancelled) {
						monitor_status();
						pad_center(Buffer[3], "Refill: Final Boost");
						update_display_chunks(Buffer);
						uint32_t end_time = HAL_GetTick() + 5000;
						while (HAL_GetTick() < end_time) {
							if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_6) == GPIO_PIN_SET) refill_button_was_released = 1;
							if (refill_button_was_released && HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_6) == GPIO_PIN_RESET) {
								cancelled = 1;
								break;
							}
							current_psi = read_pressure_psi();
							snprintf(message, sizeof(message), "%d PSI", current_psi);
							pad_center(Buffer[1], message);
							update_display_chunks(Buffer);
							HAL_Delay(100);
						}
					}

					// Always close the valve after refill attempt
					HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);
					HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_RESET);  // pump off
					HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);  // LED off

					if (cancelled) pad_center(Buffer[3], "Refill: Cancelled");
					else pad_center(Buffer[3], "Refill: Complete");

					snprintf(message, sizeof(message), "%d PSI", current_psi);
					pad_center(Buffer[1], message);
					update_display_chunks(Buffer);

					if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_6) == GPIO_PIN_SET) {
						refill_button_was_down = 0;
					}
				}

				if (refill_button == GPIO_PIN_SET) refill_button_was_down = 0;

				if (HAL_GetTick() - previousTick >= 1000) {
					HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_10);
					previousTick = HAL_GetTick();
				}

				update_display_chunks(Buffer);
			}
			if (page == 1) {
			    pad_center(Buffer[0], "Settings");

			    while (1) {  // Stay here until button press
			        // Fill Buffer
			        for (int i = 0; i < 3; i++) {
			            int option_idx = settings_scroll + i;
			            if (option_idx < settings_count) {
			                if (option_idx == settings_selection) {
			                    char temp[20];
			                    snprintf(temp, sizeof(temp), ">%s", settings_options[option_idx]);
			                    pad_left_20(Buffer[i + 1], temp);
			                } else {
			                    pad_left_20(Buffer[i + 1], settings_options[option_idx]);
			                }
			            } else {
			                pad_left_20(Buffer[i + 1], "");
			            }
			        }

			        update_display_chunks(Buffer);

			        // Handle emergency
					monitor_status();

			        // Handle encoder rotation
			        int delta = read_encoder_delta();
			        if (delta != 0) {
			            settings_selection += delta * -1;

			            if (settings_selection < 0) settings_selection = 0;
			            if (settings_selection >= settings_count) settings_selection = settings_count - 1;

			            if (settings_selection < settings_scroll) {
			                settings_scroll = settings_selection;
			            }
			            else if (settings_selection >= settings_scroll + 3) {
			                settings_scroll = settings_selection - 2;
			                if (settings_scroll > settings_count - 3) {
			                    settings_scroll = settings_count - 3;
			                    if (settings_scroll < 0) settings_scroll = 0;
			                }
			            }

//			            clear_buffer();
			        }

			        // ======= Button handling: (THIS IS THE IMPORTANT PART) =======

			        // PA4 pressed (encoder button) → select setting
			        if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4) == GPIO_PIN_RESET && pressed == 0) {
			            pressed = 1;
			            page = settings_selection + 2; // Enter selected setting page
			            break;
			        }

			        // PB5 pressed (exit button) → go back to home
			        if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_5) == GPIO_PIN_RESET && pressed == 0) {
			            pressed = 1;
			            page = 0; // Back to Home Page
			            break;
			        }

			        // Debounce release
			        if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4) == GPIO_PIN_SET && HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_5) == GPIO_PIN_SET) {
			            pressed = 0;
			        }

			        HAL_Delay(1);
			    }

			    clear_buffer();
			    update_display_chunks(Buffer);
			}




			if (page == 2) {
				pad_center(Buffer[0], "Settings: Shot Power");
				snprintf(message, sizeof(message), "Min: %d, Max: %d", shot_min, shot_max);
				pad_left_20(Buffer[1], message);
				snprintf(message, sizeof(message), "Current Level: %d", shot_length);
				pad_left_20(Buffer[2], message);
				update_display_chunks(Buffer);

				while (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4) || pressed == 1) {
					if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4) == GPIO_PIN_SET){
						pressed = 0;
					}
					monitor_status();
					int delta = read_encoder_delta();
					if (delta != 0) {
						shot_length += delta * 20 * -1;
						if (shot_length < shot_min) shot_length = shot_min;
						if (shot_length > shot_max) shot_length = shot_max;
						snprintf(message, sizeof(message), "Current Level: %d", shot_length);
						pad_left_20(Buffer[2], message);
						update_display_chunks(Buffer);
					}
					HAL_Delay(1);
				}

				page = 1;
				clear_buffer();
				update_display_chunks(Buffer);
				pressed = 1;
			}
			if (page == 3) {
			    const char *fire_modes[] = { "Single", "Burst", "Stream" };
			    const int fire_mode_count = sizeof(fire_modes) / sizeof(fire_modes[0]);

			    pad_center(Buffer[0], "Settings: Firing Mode");
			    snprintf(message, sizeof(message), "Mode: %s", fire_modes[fire_type]);
			    pad_center(Buffer[2], message);
			    update_display_chunks(Buffer);

			    while (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4) || pressed == 1) {
			        if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4) == GPIO_PIN_SET) {
			            pressed = 0;
			        }

					monitor_status();


			        int delta = read_encoder_delta();
			        if (delta != 0) {
			            fire_type += delta * -1;

			            // Wrap within 0 to fire_mode_count - 1
			            if (fire_type < 0) fire_type = 0;
			            if (fire_type >= fire_mode_count) fire_type = fire_mode_count - 1;

			            snprintf(message, sizeof(message), "Mode: %s", fire_modes[fire_type]);
			            pad_center(Buffer[2], message);
			            update_display_chunks(Buffer);
			        }

			        HAL_Delay(1);
			    }

			    page = 1;
			    clear_buffer();
			    update_display_chunks(Buffer);
			    pressed = 1;
			}
			if (page == 5) {
			    const char *sensor_options[] = { "Disabled", "Enabled" };
			    const int sensor_option_count = sizeof(sensor_options) / sizeof(sensor_options[0]);

			    pad_center(Buffer[0], "Settings: Sensors");
			    snprintf(message, sizeof(message), "Status: %s", sensor_options[use_sensors]);
			    pad_center(Buffer[2], message);
			    update_display_chunks(Buffer);

			    while (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4) || pressed == 1) {
			        if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4) == GPIO_PIN_SET) {
			            pressed = 0;
			        }

			        if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_4) == GPIO_PIN_RESET) {
			            enter_emergency_shutdown();
			        }

			        int delta = read_encoder_delta();
			        if (delta != 0) {
			            use_sensors += delta * -1;

			            // Clamp between 0 and 1
			            if (use_sensors < 0) use_sensors = 0;
			            if (use_sensors > 1) use_sensors = 1;

			            snprintf(message, sizeof(message), "Status: %s", sensor_options[use_sensors]);
			            pad_center(Buffer[2], message);
			            update_display_chunks(Buffer);
			        }

			        HAL_Delay(1);
			    }

			    page = 1;
			    clear_buffer();
			    update_display_chunks(Buffer);
			    pressed = 1;
			}
			if (page == 6) {
			    const char *bypass_options[] = { "Disabled", "Enabled" };
			    const int bypass_option_count = sizeof(bypass_options) / sizeof(bypass_options[0]);

			    pad_center(Buffer[0], "Settings: Fire Bypass");
			    snprintf(message, sizeof(message), "Status: %s", bypass_options[ignore_shots_remaining]);
			    pad_center(Buffer[2], message);
			    update_display_chunks(Buffer);

			    while (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4) || pressed == 1) {
			        if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4) == GPIO_PIN_SET) {
			            pressed = 0;
			        }

			        if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_4) == GPIO_PIN_RESET) {
			            enter_emergency_shutdown();
			        }

			        int delta = read_encoder_delta();
			        if (delta != 0) {
			            ignore_shots_remaining += delta * -1;

			            // Clamp between 0 and 1
			            if (ignore_shots_remaining < 0) ignore_shots_remaining = 0;
			            if (ignore_shots_remaining > 1) ignore_shots_remaining = 1;

			            snprintf(message, sizeof(message), "Status: %s", bypass_options[ignore_shots_remaining]);
			            pad_center(Buffer[2], message);
			            update_display_chunks(Buffer);
			        }

			        HAL_Delay(1);
			    }

			    page = 1;
			    clear_buffer();
			    update_display_chunks(Buffer);
			    pressed = 1;
			}

		}
	}
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc1.Init.LowPowerAutoWait = DISABLE;
  hadc1.Init.LowPowerAutoPowerOff = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc1.Init.SamplingTimeCommon1 = ADC_SAMPLETIME_1CYCLE_5;
  hadc1.Init.SamplingTimeCommon2 = ADC_SAMPLETIME_1CYCLE_5;
  hadc1.Init.OversamplingMode = DISABLE;
  hadc1.Init.TriggerFrequencyMode = ADC_TRIGGER_FREQ_HIGH;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLINGTIME_COMMON_1;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5|GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9
                          |GPIO_PIN_10|GPIO_PIN_12, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);

  /*Configure GPIO pins : PB9 PB3 PB4 PB5
                           PB6 PB7 PB8 */
  GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5
                          |GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PC14 PC15 */
  GPIO_InitStruct.Pin = GPIO_PIN_14|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PA1 PA2 PA3 PA4
                           PA6 PA11 PA15 */
  GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4
                          |GPIO_PIN_6|GPIO_PIN_11|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PA5 PA7 PA8 PA9
                           PA10 PA12 */
  GPIO_InitStruct.Pin = GPIO_PIN_5|GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9
                          |GPIO_PIN_10|GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB0 PB1 PB2 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PC6 */
  GPIO_InitStruct.Pin = GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
      /* User can add his own implementation to report the HAL error return state */
      __disable_irq();
      while (1) {}
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
      /* User can add his own implementation to report the file name and line number,
         ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
