/*
 * Tencent is pleased to support the open source community by making IoT Hub available.
 * Copyright (C) 2016 Tencent. All rights reserved.

 * Licensed under the MIT License (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://opensource.org/licenses/MIT

 * Unless required by applicable law or agreed to in writing, software distributed under the License is
 * distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef IOT_DATA_TEMPLATE_CLIENT_COMMON_H_
#define IOT_DATA_TEMPLATE_CLIENT_COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "data_template_client.h"

/**
 * @brief 如果没有订阅delta主题, 则进行订阅, 并注册相应设备属性
 *
 * @param pTemplate   template client
 * @param pProperty 设备属性
 * @param callback  相应设备属性处理回调函数
 * @return          返回QCLOUD_ERR_SUCCESS, 表示成功
 */
int template_common_register_property_on_delta(Qcloud_IoT_Template *pTemplate, DeviceProperty *pProperty, OnPropRegCallback callback);

/**
 * @brief 移除注册过的设备属性
 *
 * @param pTemplate   template client
 * @param pProperty 设备属性
 * @return          返回QCLOUD_ERR_SUCCESS, 表示成功
 */
int template_common_remove_property(Qcloud_IoT_Template *ptemplate, DeviceProperty *pProperty);

/**
 * @brief 检查注册属性是否已经存在
 *
 * @param pTemplate   template client
 * @param pProperty 设备属性
 * @return          返回 0, 表示属性不存在
 */
int template_common_check_property_existence(Qcloud_IoT_Template *ptemplate, DeviceProperty *pProperty);


#ifdef __cplusplus
}
#endif

#endif //IOT_DATA_TEMPLATE_CLIENT_COMMON_H_
