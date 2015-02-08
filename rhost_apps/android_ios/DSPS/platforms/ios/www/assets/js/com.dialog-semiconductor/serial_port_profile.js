/*
    Copyright 2013-2014, JUMA Technology

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

document.addEventListener('deviceready',function(){
	cordova.define("com.dialog-semiconductor.profile.serial_port", function(require, exports, module) {
		var BC = require("com.dialog-semiconductor.service.serial_port");
		
		var serviceUUID   = "0783b03e-8535-b5a0-7140-a304d2495cb7";
		var XON = "01";
		var XOFF = "02";
		var serial_port = {};
		var SerialPortProfile = BC.DialogSerialPortProfile = BC.Profile.extend({

			open : function(device,readCallback,flowControlCallback,successFunc,errorFunc){
				var that=this;
				device.connect(function(){
					serial_port.device = device;
					serial_port.readCallback = readCallback;
					serial_port.flowControlCallback = flowControlCallback;
					serial_port.flowControl = XON;
					device.discoverServices(function(){
						serial_port.service = device.getServiceByUUID(serviceUUID)[0];
						if (serial_port.service !== undefined){
							serial_port.service.subscribeRead(serial_port.readCallback);
							window.setTimeout(function(){
								serial_port.service.subscribeFlowControl(serial_port.flowControlCallback)
							},2000);
							successFunc();
						}else{
							errorFunc();
						}
					},function(){
						errorFunc();
						alert("discoverServices Error");
					});
					
				},errorFunc);
			},

			startReceive : function(device){	
				serial_port.service = device.getServiceByUUID(serviceUUID)[0];
				serial_port.service.subscribeRead(serial_port.readCallback);
			},

			stopReceive : function(){
				serial_port.service.unsubscribeRead();
			},

			close : function(device,successFunc,errorFunc){
					if (serial_port.service !== undefined){
					serial_port.service.unsubscribeRead(function(){
						serial_port.service.unsubscribeFlowControl(function(){
							serial_port = {};
							device.disconnect(function(){
								successFunc();
							},errorFunc);
						});
					});
				}else{
					device.disconnect(function(){
								successFunc();
							},errorFunc);
				}
			},

			read : function(device,successFunc,errorFunc){
			},
			
			write : function(writeType,writeValue,successFunc,errorFunc){
				if(serial_port.service){
					if (serial_port.flowControl == XON){
						serial_port.service.write(writeType,writeValue,successFunc,errorFunc);
					}else{
						errorFunc();
					}
				}
			},
			writeFlowControl : function(writeType,writeValue,successFunc,errorFunc){
				if(serial_port.service){
					serial_port.service.writeFlowControl(writeType,writeValue,successFunc,errorFunc);
				}
			}
		});
		
	
		module.exports = BC;

	});
},false);
