cordova.define('cordova/plugin_list', function(require, exports, module) {
module.exports = [
    {
        "file": "plugins/org/www/org.underscorejs.underscore/underscore.js",
        "id": "org.underscorejs.underscore"
    },
    {
        "file": "plugins/org/www/org.bcsphere/bc.js",
        "id": "org.bcsphere.bcjs"
    },
    {
        "file": "plugins/org/www/org.bcsphere.bluetooth/bluetoothapi.js",
        "id": "org.bcsphere.bluetooth.bluetoothapi",
        "merges": [
            "navigator.bluetooth"
        ]
    },
    {
        "file": "plugins/org/www/org.bluetooth.profile/proximity.js",
        "id": "org.bluetooth.profile.proximity",
        "merges": [
            "BC"
        ]
    },
    {
        "file": "plugins/org/www/org.bluetooth.profile/find_me.js",
        "id": "org.bluetooth.profile.find_me",
        "merges": [
            "BC"
        ]
    },
    {
        "file": "plugins/org/www/org.bluetooth.profile/serial_port.js",
        "id": "org.bluetooth.profile.serial_port",
        "merges": [
            "BC"
        ]
    },
    {
        "file": "plugins/org/www/org.bluetooth.service/battery_service.js",
        "id": "org.bluetooth.service.battery_service",
        "merges": [
            "BC"
        ]
    },
    {
        "file": "plugins/org/www/org.bluetooth.service/blood_pressure.js",
        "id": "org.bluetooth.service.blood_pressure",
        "merges": [
            "BC"
        ]
    },
    {
        "file": "plugins/org/www/org.bluetooth.service/health_thermometer.js",
        "id": "org.bluetooth.service.health_thermometer",
        "merges": [
            "BC"
        ]
    },
    {
        "file": "plugins/org/www/org.bluetooth.service/immediate_alert.js",
        "id": "org.bluetooth.service.immediate_alert",
        "merges": [
            "BC"
        ]
    },
    {
        "file": "plugins/org/www/org.bluetooth.service/link_loss.js",
        "id": "org.bluetooth.service.link_loss",
        "merges": [
            "BC"
        ]
    },
    {
        "file": "plugins/org/www/org.bluetooth.service/serial_port.js",
        "id": "org.bluetooth.service.serial_port",
        "merges": [
            "BC"
        ]
    },
    {
        "file": "plugins/org/www/org.bluetooth.service/tx_power.js",
        "id": "org.bluetooth.service.tx_power",
        "merges": [
            "BC"
        ]
    },
    {
        "file": "plugins/org/www/org.bcsphere.ibeacon/ibeaconapi.js",
        "id": "org.bcsphere.ibeacon.ibeaconapi",
        "merges": [
            "navigator.ibeacon"
        ]
    },
    {
        "file": "plugins/org/www/org.bcsphere/ibeacon.js",
        "id": "org.bcsphere.ibeacon",
        "merges": [
            "BC"
        ]
    },
    {
        "file": "plugins/org.apache.cordova.file/www/DirectoryEntry.js",
        "id": "org.apache.cordova.file.DirectoryEntry",
        "clobbers": [
            "window.DirectoryEntry"
        ]
    },
    {
        "file": "plugins/org.apache.cordova.file/www/DirectoryReader.js",
        "id": "org.apache.cordova.file.DirectoryReader",
        "clobbers": [
            "window.DirectoryReader"
        ]
    },
    {
        "file": "plugins/org.apache.cordova.file/www/Entry.js",
        "id": "org.apache.cordova.file.Entry",
        "clobbers": [
            "window.Entry"
        ]
    },
    {
        "file": "plugins/org.apache.cordova.file/www/File.js",
        "id": "org.apache.cordova.file.File",
        "clobbers": [
            "window.File"
        ]
    },
    {
        "file": "plugins/org.apache.cordova.file/www/FileEntry.js",
        "id": "org.apache.cordova.file.FileEntry",
        "clobbers": [
            "window.FileEntry"
        ]
    },
    {
        "file": "plugins/org.apache.cordova.file/www/FileError.js",
        "id": "org.apache.cordova.file.FileError",
        "clobbers": [
            "window.FileError"
        ]
    },
    {
        "file": "plugins/org.apache.cordova.file/www/FileReader.js",
        "id": "org.apache.cordova.file.FileReader",
        "clobbers": [
            "window.FileReader"
        ]
    },
    {
        "file": "plugins/org.apache.cordova.file/www/FileSystem.js",
        "id": "org.apache.cordova.file.FileSystem",
        "clobbers": [
            "window.FileSystem"
        ]
    },
    {
        "file": "plugins/org.apache.cordova.file/www/FileUploadOptions.js",
        "id": "org.apache.cordova.file.FileUploadOptions",
        "clobbers": [
            "window.FileUploadOptions"
        ]
    },
    {
        "file": "plugins/org.apache.cordova.file/www/FileUploadResult.js",
        "id": "org.apache.cordova.file.FileUploadResult",
        "clobbers": [
            "window.FileUploadResult"
        ]
    },
    {
        "file": "plugins/org.apache.cordova.file/www/FileWriter.js",
        "id": "org.apache.cordova.file.FileWriter",
        "clobbers": [
            "window.FileWriter"
        ]
    },
    {
        "file": "plugins/org.apache.cordova.file/www/Flags.js",
        "id": "org.apache.cordova.file.Flags",
        "clobbers": [
            "window.Flags"
        ]
    },
    {
        "file": "plugins/org.apache.cordova.file/www/LocalFileSystem.js",
        "id": "org.apache.cordova.file.LocalFileSystem",
        "clobbers": [
            "window.LocalFileSystem"
        ],
        "merges": [
            "window"
        ]
    },
    {
        "file": "plugins/org.apache.cordova.file/www/Metadata.js",
        "id": "org.apache.cordova.file.Metadata",
        "clobbers": [
            "window.Metadata"
        ]
    },
    {
        "file": "plugins/org.apache.cordova.file/www/ProgressEvent.js",
        "id": "org.apache.cordova.file.ProgressEvent",
        "clobbers": [
            "window.ProgressEvent"
        ]
    },
    {
        "file": "plugins/org.apache.cordova.file/www/fileSystems.js",
        "id": "org.apache.cordova.file.fileSystems"
    },
    {
        "file": "plugins/org.apache.cordova.file/www/requestFileSystem.js",
        "id": "org.apache.cordova.file.requestFileSystem",
        "clobbers": [
            "window.requestFileSystem"
        ]
    },
    {
        "file": "plugins/org.apache.cordova.file/www/resolveLocalFileSystemURI.js",
        "id": "org.apache.cordova.file.resolveLocalFileSystemURI",
        "merges": [
            "window"
        ]
    },
    {
        "file": "plugins/org.apache.cordova.file/www/android/FileSystem.js",
        "id": "org.apache.cordova.file.androidFileSystem",
        "merges": [
            "FileSystem"
        ]
    },
    {
        "file": "plugins/org.apache.cordova.file/www/fileSystems-roots.js",
        "id": "org.apache.cordova.file.fileSystems-roots",
        "runs": true
    },
    {
        "file": "plugins/org.apache.cordova.file/www/fileSystemPaths.js",
        "id": "org.apache.cordova.file.fileSystemPaths",
        "merges": [
            "cordova"
        ],
        "runs": true
    },
	{
        "file": "js/com.dialog-semiconductor/serial_port_profile.js",
        "id": "com.dialog-semiconductor.profile.serial_port",
        "merges": [
            "BC"
        ]
    },
    {
        "file": "js/com.dialog-semiconductor/serial_port_service.js",
        "id": "com.dialog-semiconductor.service.serial_port",
        "merges": [
            "BC"
        ]
    },
    {
      "file": "plugins/org.bcsphere.cordova.utilities/bcutility.js",
      "id": "org.bcsphere.cordova.utilities",
      "merges": [
                 "BCUtility"
                 ]
    },
];
module.exports.metadata = 
// TOP OF METADATA
{
    "org": "0.5.0",
    "org.apache.cordova.file": "1.3.1"
}
// BOTTOM OF METADATA
});