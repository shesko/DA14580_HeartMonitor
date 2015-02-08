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

var app = {
	MTU : {
		"android":128,
		"ios":128
	},

	// global
	device : {},
	running_platform:null,
	flowcontrol_data:1,
	g_connected : 0,
	ftx_mode:0,
	device_counter:0,

	// rx
	dataBuffer0:[],
	dataBuffer1:[],
    dataStr1:"",
    dataStr2:"",
	end_data_flag:1,

	// file tx
	ftx_interval:5,
	ftx_count:0,
	count_rem_data:0,
	count_rem_tag:1,
	
	// cyclic tx
	cycle_tx_stop : 0,
	cycle_tx_ongoing : 1,
	cycle_tx_count : 0,
	cycle_tx_timer : null,
	arti_disconnect : 0,

	timer:null,
	tag:0,
	reloadCount:true,
	searchAgainTimer:null,
	consoleCounter:true,


	//rss
	RSSIrange:[[-100,-90],[-89,-80],[-79,-70],[-60,-40],[-40,20]],
	//switchRSSI
	switchRSSI:function(RSSI){
		for(var i=0,len=app.RSSIrange.length;i<len;i++){
			var tempArr=app.RSSIrange[i];
			if(RSSI<=tempArr[1]){
				return i;
				break;
			}
		}
	},

    initialize : function() {
        app.bindCordovaEvents();
        app.setTextAreaH();
        app.about();
    },
    
    bindCordovaEvents : function() {
		document.addEventListener('deviceready',app.onDeviceReady,false);
        document.addEventListener('bcready', app.onBCReady, false);
    },
    
	onDeviceReady : function(){
        var BC = window.BC = cordova.require("com.dialog-semiconductor.profile.serial_port");
        var ProgressEvent = window.ProgressEvent = cordova.require('org.apache.cordova.file.ProgressEvent');
        var LocalFileSystem = window.LocalFileSystem = cordova.require('org.apache.cordova.file.LocalFileSystem');
        var requestFileSystem = window.requestFileSystem = cordova.require('org.apache.cordova.file.requestFileSystem');
        var FileReader = window.FileReader = cordova.require('org.apache.cordova.file.FileReader');
        var BCUtility = window.BCUtility = cordova.require('org.bcsphere.cordova.utilities');
    },
	
	onBCReady : function(){
		app.bindUIEvent();
		app.running_platform = app.testPlatform();

		app.DialogSerialPortProfile = new BC.DialogSerialPortProfile();

		if(BC.bluetooth.isopen){
		    app.onBluetoothScan();
		}else{
		    alert("please turn on bluetooth !");
		    BC.bluetooth.addEventListener("bluetoothstatechange",app.onBluetoothScan);
		}	
	},

	onBluetoothScan:function(){
		BC.bluetooth.addEventListener("newdevice",app.addNewDevice);
	    BC.Bluetooth.StartScan();
	    var deviceList = document.getElementById('deviceList');
		var deviceListH = window.innerHeight - app.offset(deviceList).top;
		var top = document.getElementById('circlePic').offsetHeight;
		
		var filterInterVal = setInterval(function() {
			if (deviceList.offsetHeight > deviceListH) {
				window.clearInterval(filterInterVal);
				$("#circlePic").css("display", "none");
				$(".searchRotate").css("display", "block");
				$("#deviceList").css("margin-top",top).animate({
					'margin-top': 0
				}, 800,function(){
					deviceList.style.height = window.innerHeight - app.offset(deviceList).top + "px";
				});
				deviceList.style.overflowX = "hidden";
				deviceList.style.overflowY = "auto";
			}
		}, 100);
		app.stopScan();	
	},

	testPlatform : function (){
		if (/android/i.test(navigator.userAgent)){
		    return "android";
		}else if (/ipad|iphone|mac/i.test(navigator.userAgent)){
		    return "ios"
		}else{
			throw new Error("Neither Android nor IOS platform");
		}
	},

	bindUIEvent : function(){
		$('#reload').click(function() {
			$(".searchRotate").children('span').html("0 device found");
			$(".searchRotate").children('img').attr('src', 'img/searchRotate.png')

			var deviceInfo = $('#deviceItem').siblings().remove();

			app.device_counter = 0;
			app.reloadCount=false;

			BC.bluetooth.devices = {};
			BC.Bluetooth.StopScan();
			BC.Bluetooth.StartScan();
			app.stopScan();

		});
	},

	offset : function(ele) {
		var l = ele.offsetLeft;
		var t = ele.offsetTop;
		var p = ele.offsetParent;

		while (p) {
			l += p.offsetLeft + p.clientLeft;
			t += p.offsetTop + p.clientTop;
			p = p.offsetParent;
		}
			
		return {
			top: t,
			left: l
		};
	},

	addNewDevice : function(data){

		var deviceFoundStr=(++app.device_counter)>1?" devices found":"device found";
		$(".searchRotate").children('span').html(app.device_counter +" "+ deviceFoundStr);

		var device = data.target;
		var listGroup = $('#deviceList');
		var deviceInfo = $('#deviceItem').clone();
		if(device.deviceName){
			$('#deviceName',deviceInfo).html(device.deviceName);
		}else{
			$('#deviceName',deviceInfo).html('Unknown')
		}
		$('#deviceAddress',deviceInfo).html(device.deviceAddress);
		$('.wrap_active',deviceInfo).addClass("showActive"+app.switchRSSI(device.RSSI));
		deviceInfo.show();
		deviceInfo.click(function(){			
			if(device.deviceName){
				$(".navigation-bar__center>.deviceName").html(device.deviceName);
			}else{
				$(".navigation-bar__center>.deviceName").html("none");
			}

			$(".navigation-bar__center>.deviceAddress").html(device.deviceAddress);
			BC.Bluetooth.StopScan();

			if(app.reloadCount){
				app.reloadCount=false;
				$(".search").css({
					display: "block",
					position: "absolute",
					top: window.innerHeight + "px"
				}).animate({
					top: (window.innerHeight - $('.search').height()) + "px"
				}, 500,function(){
					$(".searchRotate").children('img').attr('src', 'img/stopSearch.png');
				});
			}

			$.mobile.changePage("#mainPage");
			app.start(device.deviceAddress);
		});
		listGroup.append(deviceInfo);
	},

	start:function(deviceAddress){
		app.device = BC.bluetooth.devices[deviceAddress];
		if(!app.arti_disconnect){
			app.device.addEventListener("devicedisconnected",app.onDeviceDisconnected,false);
		}
		app.deviceOpen();
		if(app.consoleCounter){
			app.consoleMode(); 
			app.consoleCounter=false;
		}
	},

	//go back to scanpage
	backToScanPage : function(){
        var fileList = document.getElementById('fileList');
        fileList.innerHTML = "";
		$.mobile.changePage("#scanPage");
	},

	setTextAreaH:function(){

		var inputInfoH=Math.floor(window.innerHeight-document.getElementsByClassName("btn")[2].offsetHeight-document.getElementsByClassName("btn")[3].offsetHeight-document.getElementsByClassName("tab_bar")[0].offsetHeight-document.getElementsByClassName("navigation-bar")[0].offsetHeight-8);

		var inputInfos=document.getElementsByClassName("inputInfo");
		for(var i=0;i<inputInfos.length;i++){
			if(i%2==0){
				inputInfos[i].style.height=Math.floor(inputInfoH/4*1.2)+"px";
			}else{
				inputInfos[i].style.height=Math.floor(inputInfoH/4*0.8)+"px";
			}
		}

		$(".tabContent").each(function(index,val){
			if(index==0){
				$(val).addClass("selected");
			}else{
				$(val).removeClass("selected");
			}
		})

		$(".tabOption>li").each(function(index,val){
			if(index==0){
				$(val).removeClass("defaultLi").addClass("selectedLi");
			}else{
				$(val).removeClass("selectedLi").addClass("defaultLi");
			}
		})
	},

	stopScan : function(){
		window.clearTimeout(app.searchAgainTimer);
		app.searchAgainTimer=setTimeout(function(){
			BC.Bluetooth.StopScan();
			

			$("#circlePic").css("display", "none");
			$(".searchRotate").css("display", "block");
			if($(".item").length>1){
				$(".item").css("marginBottom",0).last().css("marginBottom","7.5em");
			}
			//items[items.length - 1].style.marginBottom = "7.5em";

			if(app.reloadCount){
				$(".search").css({
					display: "block",
					position: "absolute",
					top: window.innerHeight + "px"
				}).animate({
					top: (window.innerHeight - $('.search').height()) + "px"
				}, 800,function(){
					$(".searchRotate").children('img').attr('src', 'img/stopSearch.png');
				});
			}else{
				$(".searchRotate").children('img').attr('src', 'img/stopSearch.png');
			}
		},5*1000);
	},

	disconnectSuccessFunc : function(){
		console.log("disconnect success");
		//disconnect occur ,clear all window
		app.clearShowData();
		app.clearConsoleModeData();
		app.clearWriteValue();
		app.onDeviceDisconnected_signal();
	},	
	disconnectErroFunc:function(){
		console.log("disconnect fail");
	},

	//scan page disconnect
	scanPageDisconnect : function(){
		app.arti_disconnect = 1;
		app.DialogSerialPortProfile.close(app.device,app.disconnectSuccessFunc,app.disconnectErroFunc);
	},
	//scan page exit
	scanPageExit : function(){
		BCUtility.exitApp();
	},

    /*---------------------------------------------------------------------------------------------------------------------------------------------*/
    // GAP
	onDeviceDisconnected: function(arg){
		if(!app.arti_disconnect){
			app.g_connected = 0;
			$(".connectStatus").removeClass("connected").addClass("disconnected");
			alert("device:"+arg.deviceAddress+" is disconnected!");
			app.deviceOpen();
			setTimeout(function(){
					if(arg.isConnected){
						console.log("connect success");
						app.onStartReceive();	
					}else{
						console.log("lose!!");
					}
			},1000);
		}
	},

	onDeviceDisconnected_signal: function(){
		app.g_connected = 0;
		$(".connectStatus").removeClass("connected").addClass("disconnected");
	},

	openSuccessFunc : function(){
		app.arti_disconnect = 0;
		app.g_connected = 1;
		$(".connectStatus").removeClass("disconnected").addClass("connected");
		console.log("connect success");
	},	
	openErroFunc:function(){
		app.g_connected = 0;
		console.log("connect fail");
	},
	deviceOpen : function(){
		requestFileSystem(LocalFileSystem.PERSISTENT, 0, app.gotFS, app.fail);
		app.DialogSerialPortProfile.open(app.device,app.defaultReceive,app.flowControlCallback,app.openSuccessFunc,app.openErroFunc);
	},
	

    /*---------------------------------------------------------------------------------------------------------------------------------------------*/
    //Log file

    gotFS:function(fileSystem){
        newFile = fileSystem.root.getDirectory("DSPS", {create : true,exclusive : false}, app.writerFile, app.fail);
        app.fileSystem=fileSystem;
        document.getElementById("Directory").addEventListener("touchstart",app.readDirectory,false);
    },

    readDirectory:function(){
        app.readEntries(app.fileSystem.root);
    },
    writerFile : function(newFile){

    	var fileName = 'DA14580-01_';
    	var date = new Date();
    	fileName+= date.getFullYear();
		fileName+=parseInt(date.getMonth()+1)>9?parseInt(date.getMonth()+1).toString():'0' + parseInt(date.getMonth()+1);
		fileName+=date.getDate()>9?date.getDate().toString():'0' + date.getDate();
		fileName+=date.getHours()>9?date.getHours().toString():'0' + date.getHours();
		fileName+=date.getMinutes()>9?date.getMinutes().toString():'0' + date.getMinutes();
		fileName+=date.getSeconds()>9?date.getSeconds().toString():'0' + date.getSeconds();
		fileName+='_'+app.deviceAddressToString(app.device.deviceAddress);
		fileName+='.txt';
        newFile.getFile(fileName, {create : true,exclusive : false}, app.gotFileEntry, app.fail);  
    },

    deviceAddressToString:function(deviceAddress){
    	while(deviceAddress.indexOf(':')!=-1){
    		deviceAddress = deviceAddress.replace(':','')
    	}
    	return deviceAddress;
    },

    write : function(mes){	
		app.writer.seek(app.writer.length);		
		app.writer.write(mes);
    },

     gotFileEntry : function(fileEntry){
        fileEntry.createWriter(app.gotFileWriter, app.fail);
    },

     gotFileWriter: function(writer) {  
        writer.onwrite = function(evt) { 
            console.log("write success");             
        };
        writer.onabort = function(evt) { 
            console.log("write onabort");  
        };
        writer.onerror = function(evt) { 
            console.log("write onerror");  
        };
        writer.onwriteend = function(evt) { 
        	console.log("write end");
        	if(app.tag==1){
                app.dataStr1="";
        		app.dataBuffer0=[];
        	}else{
                app.dataStr2="";
        		app.dataBuffer1=[];
        	}
              
        };
        app.writer = writer;
    },
 
 	/*---------------------------------------------------------------------------------------------------------------------------------------------*/
 	//Read file for file data streaming   
    gotFiles:function(entries){
  
        var length = entries.length;
        var fileList = document.getElementById('fileList');
        fileList.innerHTML = "";
        if(length>0){
                entries[0].getParent(function(parent){
                    if(parent.name!= app.fileSystem.root.name){
                        var parentItem = document.createElement('li');
                        parentItem.innerHTML ="<strong>..</strong>";
                        parentItem.onclick = function(){
                            parent.getParent(function(pare){
                                app.readEntries(pare);
                            });
                        }
                        fileList.appendChild(parentItem);
                    }
                    for(var i=0;i<length;i++){
                        var fileEntry = entries[i];
                        var fileItem = document.createElement('li');
                        if(fileEntry.isFile){
                            fileItem.innerHTML = fileEntry.name+'[F]';
                            ;(function(fileEntry){
                                fileItem.onclick = function(){
                                	var conf=confirm("Start file data streaming");
                                	if(conf){
                                    	app.ftx_interval = parseInt($('#connectionInterval').val())||0;
	                                	if(cordova.platformId == "ios"){
	                                		if (app.ftx_interval < 30)
	                                		app.ftx_interval = 30;
	                                	}else{
	                                	    if(app.ftx_interval < 5)
	                                		app.ftx_interval = 5;
	                                	}
	                                    app.readFileAsText(fileEntry);
                                    }
                                };
                            })(fileEntry);
                        }else{
                            fileItem.innerHTML = fileEntry.name+'[D]';
                            ;(function(fileEntry){
                                fileItem.onclick = function(){
                                    app.readEntries(fileEntry);
                                };
                            })(fileEntry);
                        }
                        fileList.appendChild(fileItem);
                    }
                });
        }else{
            if(app.fileEntry!=null){
                var parentItem = document.createElement('li');
                    parentItem.innerHTML ="<strong>..</strong>";
                    parentItem.onclick = function(){
                        app.fileEntry.getParent(function(pare){
                            app.readEntries(pare);
                        });
                    }
                fileList.appendChild(parentItem);
            };
        }
    },

     readEntries : function(fileEntry){
        app.fileEntry = fileEntry;
        var dirReader = fileEntry.createReader();
        dirReader.readEntries(app.gotFiles,function(){console.log("doDirectoryListing error callback")});
    },

    readFileAsText : function(fileEntry){
    	fileEntry.file(function(file){
           	var reader = new FileReader();
           	reader.onloadend = function(e){
               	var data=e.target.result;
                if(data.length>0){
                	app.ftx_mode = 1;
               		app.writeDataMTU("ASCII",data,function(){console.log("write success")},function(){console.log("write error")});
               	}		
            }
            reader.error=function(){
            }
           
            reader.readAsText(file);
        },function(e){alert(e.code)});
    },

    fail : function(error){
        console.log("Failed to retrieve file:" + error.code);  
    },


    /*---------------------------------------------------------------------------------------------------------------------------------------------*/
    //rx receive
    defaultReceive : function(data){
    	if(app.timer!=null){
    		clearTimeout(app.timer);
    	}
    	var nIndex=$(".tabOption>li").index($(".selectedLi"));
    	if(nIndex!==0){
    		//update rx window
    		var hexChecked = document.getElementById("RxHex").checked;
    		var showData = document.getElementById("showData");
    		var func = hexChecked?"getHexString":"getASCIIString";
	        var displaydata = showData.innerHTML+data.value[func]();
	        if(displaydata.length>350){	        	
	        	showData.innerHTML = displaydata.slice(displaydata.length-350);
	        }else{
	        	showData.innerHTML = displaydata;
	        }	
    	}else{
    		// update console window
    		var hexChecked = document.getElementById("consolerxHex").checked;
    		var consoleModeData = document.getElementById("consoleModeData");
    		var func = hexChecked?"getHexString":"getASCIIString";
    		var displaydata = consoleModeData.innerHTML+data.value[func]();
    		if(displaydata.length>350){   			
	        	consoleModeData.innerHTML = displaydata.slice(displaydata.length-350);
	        }else{
	        	consoleModeData.innerHTML = displaydata;
	        }
    	}
        
    	if(app.tag==0){
            app.dataStr1=app.dataStr1+data.value.getASCIIString();
    	}else{
            app.dataStr2=app.dataStr2+data.value.getASCIIString();
    	}
        
    	if((app.dataStr1.length>=6000) && (app.tag == 0)){
    		app.tag=1;
            app.write(app.dataStr1);
            app.dataStr1="";
   		}else if((app.dataStr2.length>=6000) && (app.tag==1)){
          app.tag=0;
          app.write(app.dataStr2);
          app.dataStr2 ="";
   		}
   		if(app.end_data_flag == 1){
	    	app.timer=setTimeout(function(){
	    		if(app.tag==0){
                        app.write(app.dataStr1);
                        app.dataStr1="";
	    		}else{
                        app.write(app.dataStr2);
                        app.dataStr2 ="";
	    		}
	    	},1000);
		}
     
        data = null
   },

    /*---------------------------------------------------------------------------------------------------------------------------------------------*/
    //rx send flow contol 
	onStartReceive : function(){
		app.end_data_flag = 1;
		app.DialogSerialPortProfile.writeFlowControl("Hex","01",function(){console.log("hex 01")},function(){console.log("hex 01 error")});
	},

	onStopReceive : function(){
		app.end_data_flag = 0;
		app.DialogSerialPortProfile.writeFlowControl("Hex","02",function(){console.log("hex 02")},function(){console.log("hex 02 error")});
	},

	//clear rx window
	clearShowData : function(){
		document.getElementById("showData").innerHTML="";
	},
	//clear tx window
	clearWriteValue : function(){
		document.getElementById("WriteValue").value="";
		document.getElementById("connectionInterval").value="";
	},
	//clear console window
	clearConsoleModeData : function(){
		document.getElementById("consoleModeData").innerHTML = "";
		document.getElementById("consoleMode").value = "";
	},
    /*---------------------------------------------------------------------------------------------------------------------------------------------*/
    //rx receive flow control
    flowControlCallback : function(data){
    	// case 1: send big data in one time
    	// case 2: send cyclic
    	// case 3: file tx streaming

    	var XON="01";
    	var XOFF="02";
    	if(data.value.getHexString()==XOFF){
    		app.flowcontrol_data = 0;
    		console.log("XOFF");
    		window.clearInterval(app.cycle_tx_timer);
    	}else if(data.value.getHexString()==XON){
    		app.flowcontrol_data = 1;
    		console.log("XON");
    		if(!app.ftx_mode){
    		app.onSend();
    		}
    	}else{
    		alert("flowcontrol code is not correct.");
    	}
    },

    /*---------------------------------------------------------------------------------------------------------------------------------------------*/
    //tx send and stop button
	onSend : function(){
		var hexChecked = document.getElementById("TxHex").checked;
		var writeType = hexChecked?"HEX":"ASCII";

		var cycleSendChecked = document.getElementById("cycleSend").checked;
		var writeValue = document.getElementById("WriteValue").value;

		if(cycleSendChecked){
			var interval = parseInt(document.getElementById("interval").value);
			app.writeDataToDeviceCycle(writeType,writeValue,interval);
		}else{
			app.writeDataToDevice(writeType,writeValue);
		}
	},

	onStop : function(){
		window.clearInterval(app.cycle_tx_timer);
		app.cycle_tx_stop = 1;
	},


    /*---------------------------------------------------------------------------------------------------------------------------------------------*/
    // tx data for one time
	writeDataToDevice : function(writeType,writeValue){
		window.clearInterval(app.cycle_tx_timer);
		app.writeDataMTU(writeType,writeValue);
	},

    /*---------------------------------------------------------------------------------------------------------------------------------------------*/
    // cyclic tx data
	writeDataToDeviceCycle : function(writeType,writeValue,interval){
		window.clearInterval(app.cycle_tx_timer);
		app.cycle_tx_ongoing = 1;
		app.cycle_tx_stop = 0;

		app.cycle_tx_timer = window.setInterval(function(){
			if (app.g_connected==0){
				return;
			}
			// send a box of data in the tx input window
			if(app.cycle_tx_ongoing){
				app.writeCyclicDataMTU(writeType,writeValue);
			}
		},interval);
	},

	writeCyclicDataMTU : function(writeType,writeValue){
		var MTU = app.MTU[app.running_platform]
		var times = Math.ceil(writeValue.length/MTU);
		app.cycle_tx_ongoing = 0;
		app.cycle_tx_count=0;
		;(function(){
			if(app.cycle_tx_count==times){
				app.cycle_tx_ongoing = 1;
				return;
			}
			if (app.g_connected==0){
				return;
			}
			var that=arguments.callee;
			if(app.flowcontrol_data && app.cycle_tx_stop!=1){
				var curWriteValue = writeValue.slice(app.cycle_tx_count*MTU,(app.cycle_tx_count+1)*MTU);
				app.cycle_tx_count++;
				console.log(curWriteValue);
				setTimeout(function(){
					app.DialogSerialPortProfile.write(writeType,curWriteValue,that,function(){console.log("Fatal: GATT API write error!")});
				},5);
			}else{
				// the box of data will be dumped directly.
				return;
			}
		})();		

	},


    /*---------------------------------------------------------------------------------------------------------------------------------------------*/
    // file data streaming / tx data for one time
	writeDataMTU : function(writeType,writeValue){
		var MTU = app.MTU[app.running_platform]
		var times = Math.ceil(writeValue.length/MTU);
		console.log(times);
		app.ftx_count=0;
		;(function(){
			if(app.ftx_count==times){
				app.ftx_mode = 0;
				return;
			}
			if (app.g_connected==0){
				app.ftx_mode = 0;
				return;
			}
			var that=arguments.callee;
			if(app.flowcontrol_data){
				var curWriteValue = writeValue.slice(app.ftx_count*MTU,(app.ftx_count+1)*MTU);
				app.ftx_count++;
				console.log(curWriteValue);
				setTimeout(function(){
					app.DialogSerialPortProfile.write(writeType,curWriteValue,that,function(){console.log("Fatal: GATT API write error!")});
				},app.ftx_interval);
				app.count_rem_tag = 1;
			}else{
				if(app.count_rem_tag == 1){
					app.count_rem_data = writeValue.slice(app.ftx_count*MTU);
				}
				setTimeout(function(){
					app.writeDataMTU("ASCII",app.count_rem_data);
					app.count_rem_tag = 0;
				},100);
			}
		})();		
	},

	 /*---------------------------------------------------------------------------------------------------------------------------------------------*/
	//console mode
	consoleMode : function(){
		var consoleMode=document.getElementById("consoleMode");
		consoleMode.addEventListener("keydown",function(e){
			app.preData=this.value;		
		},false);
		consoleMode.addEventListener("keyup",function(e){
			app.nextData=this.value;
			if(app.preData.length<app.nextData.length){
				var data=app.nextData.slice(-1);
				if(data){
					var hexChecked=document.getElementById("consoleTxHex").checked;
					var writeType=hexChecked?"HEX":"ASCII";
	    			app.DialogSerialPortProfile.write(writeType,data,function(){console.log("write success")},function(){console.log("write error")});
	    		}
    		}    			
		},false);
	},
	sendProgress : function(curSize,allSize){
		var innerLine=document.getElementById("innerLine");
		var curProgress=Math.ceil(curSize/allSize*1000)/10+"%";
		innerLine.style.width=curProgress;
	},

	about:function(){
		var DSPSinfo=document.getElementById("DSPSinfo");
		DSPSinfo.style.cssText="position:absolute;top:50%;";
		DSPSinfo.style.marginTop=-16/2+"em";
	}
};
