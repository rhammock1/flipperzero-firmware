#include <furi.h>
#include <furi_hal.h>
#include <gui/gui.h>
#include <dolphin/dolphin.h>
#include <input/input.h>
#include <stdlib.h>
#include <stdint.h>
#include <mcp9808.h>

typedef enum {
  keyUp,
  keyRight,
  keyDown,
  keyLeft,
  keySelect,
} Key;

typedef enum {
  WaitingForKey,
  KeyPressed,
} AppState;

typedef struct {
  Key keyPressed;
  AppState state;
} TempState;

typedef enum {
  EventTypeKey,
} EventType;

typedef struct {
  EventType type;
  InputEvent input;
} TempEvent;

float temperature = 0.0;
uint32_t last_temperature_read = 0; // last temperature read time
int delay = 1000; // 1 second delay between temperature reads

static void temp_sensor_render_callback(Canvas* const canvas, void* ctx) {
  const TempState* temp_state = acquire_mutex((ValueMutex*)ctx, 25);
  if(temp_state == NULL) {
    return;
  }

  uint32_t current_tick = furi_hal_get_tick();
  if(furi_hal_i2c_is_device_ready(
    &furi_hal_i2c_handle_external, MCP9808_READ_ADDRESS, 10) && current_tick - last_temperature_read > (uint8_t)delay) {
    temperature = mcp9808_read_temp(&furi_hal_i2c_handle_external);
    last_temperature_read = current_tick;
  }
  
  canvas_draw_frame(canvas, 0, 0, 128, 64);

  canvas_set_font(canvas, FontPrimary);
  canvas_draw_str(canvas, 22, 25, "The Temperature");

  char buffer[10];
  snprintf(buffer, sizeof(buffer), "is: %0.2f", temperature);
  canvas_draw_str_aligned(canvas, 64, 41, AlignCenter, AlignBottom, buffer);


  release_mutex((ValueMutex*)ctx, temp_state);
}

static void temp_sensor_app_input_callback(InputEvent* input_event, osMessageQueueId_t event_queue) {
  furi_assert(event_queue);

  TempEvent event = {.type = EventTypeKey, .input = *input_event};
  osMessageQueuePut(event_queue, &event, 0, osWaitForever);
}

int32_t temp_sensor_app() {
  furi_hal_i2c_acquire(&furi_hal_i2c_handle_external);

  if(!furi_hal_i2c_is_device_ready(
    &furi_hal_i2c_handle_external, MCP9808_READ_ADDRESS, 10)) {
    FURI_LOG_I("TEMPSENSOR", "Device not ready");
  }

  mcp9808_init(&furi_hal_i2c_handle_external);
  osMessageQueueId_t event_queue = osMessageQueueNew(8, sizeof(TempEvent), NULL);

  TempState* temp_state = malloc(sizeof(TempState));
  ValueMutex state_mutex;
  if(!init_mutex(&state_mutex, temp_state, sizeof(TempState))) {
      printf("[TempSensorApp] cannot create mutex\r\n");
      free(temp_state);
      return 255;
  }

  ViewPort* view_port = view_port_alloc();
  view_port_draw_callback_set(view_port, temp_sensor_render_callback, &state_mutex);
  view_port_input_callback_set(view_port, temp_sensor_app_input_callback, event_queue);

  // Open GUI and register view_port
  Gui* gui = furi_record_open("gui");
  gui_add_view_port(gui, view_port, GuiLayerFullscreen);

  TempEvent event;
  temp_state->state = WaitingForKey;
  for(bool processing = true; processing;) {
    osStatus_t event_status = osMessageQueueGet(event_queue, &event, NULL, 100);
    if(event_status == osOK) {
      if(event.type == EventTypeKey) {
        if(event.input.type == InputTypePress) {
          if(event.input.key == InputKeyOk) {
            DOLPHIN_DEED(DolphinDeedTemperatureSensor);
          }
          if(event.input.key == InputKeyBack) {
            processing = false;
            break;
          }
        }
      }
    }
    view_port_update(view_port);
  }
  mcp9808_poweroff(&furi_hal_i2c_handle_external);
  furi_hal_i2c_release(&furi_hal_i2c_handle_external);
  view_port_enabled_set(view_port, false);
  gui_remove_view_port(gui, view_port);
  furi_record_close("gui");
  view_port_free(view_port);
  osMessageQueueDelete(event_queue);
  delete_mutex(&state_mutex);
  free(temp_state);

  return 0;
}
