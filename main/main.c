#include <stdio.h>
#include "string.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_event.h"
#include "nvs_flash.h"

#include "lwip/ip4_addr.h"

#include "st7789v.h"

#define WIFI_DEFAULT_SSID 			"HUAWEI-0041TC"
#define WIFI_DEFAULT_PASSWORD		"yf12345678"

char *TAG = "wifi";

static void event_handler(void* arg, esp_event_base_t event_base,int32_t event_id, void* event_data)
{
	if(event_base == WIFI_EVENT)//如果是WiFi事件
	{
		switch((uint8_t)event_id)
		{
			case WIFI_EVENT_STA_START://启动完成
				ESP_ERROR_CHECK(esp_wifi_connect());//连接WiFi
				ESP_LOGI(TAG,"connect to:%s\n",WIFI_DEFAULT_SSID);
			break;

			case WIFI_EVENT_STA_CONNECTED://连接成功
				ESP_LOGI(TAG,"connect success.\n");
			break;

			case SYSTEM_EVENT_STA_DISCONNECTED://断开连接
				ESP_LOGI(TAG,"disconnected.\n");
				ESP_ERROR_CHECK(esp_wifi_connect());//连接WiFi
			break;
		}
	}
	if(event_base == IP_EVENT)//如果是IP事件(获取到IP地址)
	{
		ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
    ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
		lcd_show_string(0,0,ip4addr_ntoa((const ip4_addr_t *)&event->ip_info.ip),COLOR_WHITE,COLOR_BLACK);
	}
}

void wifiStaInit(void)
{
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();//初始化WiFi配置为默认值
	wifi_config_t wifi_config = {
		.sta={
			.ssid = WIFI_DEFAULT_SSID,
			.password = WIFI_DEFAULT_PASSWORD,
		},
	};

	ESP_ERROR_CHECK(esp_netif_init());//初始化TCP/IP协议栈
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));//初始化WiFi
	ESP_ERROR_CHECK(esp_event_loop_create_default());//创建默认的循环事件
	esp_netif_create_default_wifi_sta();//TCP/IP协议栈配置默认
	
	ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,ESP_EVENT_ANY_ID,&event_handler,NULL,NULL));//为所有的WiFi事件注册回调
	ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,IP_EVENT_STA_GOT_IP,&event_handler,NULL,NULL));//为获取到IP的事件注册回调
	
	ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));//将WiFi信息保存到RAM
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));//配置WiFi模式
	ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA,&wifi_config));//配置连接
	ESP_ERROR_CHECK(esp_wifi_start());//启动WiFi
}

void app_main(void)
{
	//Initialize NVS
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);
	wifiStaInit();//初始化WiFi连接
	lcd_init();//初始化显示
	while (1)
	{
	  vTaskDelay(100);
	}
}
