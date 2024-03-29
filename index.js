const util = require('util');
const { spawnSync } = require('child_process');
const path = require('path');
const bleno = require('bleno');
const fs = require('fs');
// const zlib = require('zlib');
// const convertString = require('convert-string');

const SDKFile = path.join(path.resolve(__dirname, 'FingerPrintSDKSource/SoftcomFingerPrintSDK'));

const BlenoPrimaryService = bleno.PrimaryService;
const BlenoCharacteristic = bleno.Characteristic;
const BlenoDescriptor = bleno.Descriptor;


/**
 * Constructs a message as a buffer for reuse in the application.
 * @param message
 * @returns {Buffer}
 */
const constructMessage = message => new Buffer.from(message, 'utf8');
const constructHexMessage = message => new Buffer.from(message, 'hex');


const processEnrolledTemplate = async cb => {

	let file = fs.readFileSync(path.resolve(path.join(__dirname, 'tpl.bin')));
	let chunkSize = 15; // so that we can +i to make it 16bytes

	for (let i = 0; i < (file.length + chunkSize); i += chunkSize) {
		let chunk = file.slice(i, (i + chunkSize));
		cb(chunk);
	}
	// Template file sent completely.
	//cb(constructMessage('TEMPLATE COMPLETE'));

	//let templateBMP = fs.readFileSync(path.resolve(path.join(__dirname, 'tpl.bin')));

	//chunkSize = 18;

	//for (let i = 0; i < (templateBMP.length + chunkSize); i += chunkSize) {
	//	let chunk = templateBMP.slice(i, (i + chunkSize));
	//	cb(chunk);
	//}
	// Template file sent completely.
	cb(constructMessage('PROCESS COMPLETE'));

};

/**
 * Softcom Fingerprint SDK
 * @returns {{CloseDevice: (function(): Buffer), EnrollHostFinger: (function(*): Buffer), StartEnrollment: (function(): Buffer), isFingerPressed: (function(): Buffer), OpenDevice: (function(): Buffer)}}
 * @constructor
 */
const SoftcomFingerPrintSDK = () => {

	const options = {
		shell: false,
		stdio: 'pipe'
	};
	return {
		/**
		 * Opens the fingerprint device.
		 * @constructor
		 */
		OpenDevice: async () => await spawnSync(SDKFile, ['open'], options),
		/**
		 * Closes the finger print device.
		 * @constructor
		 */
		CloseDevice: async () => await spawnSync(SDKFile, ['close'], options),
		/**
		 * Checks the status of a finger press on the device.
		 * Delay is in milliseconds.
		 * @returns {Promise<*>}
		 */
		isFingerPressed: async () => await spawnSync(SDKFile, ['finger'], options),
		/**
		 * Start the enrollment process and return a "SUCCESS ENROLLMENT ::ID"
		 * @returns {Promise<Buffer>}
		 * @constructor
		 */
		StartEnrollment: async () => await spawnSync(SDKFile, ['start'], options),
		/**
		 * Enroll the capture finger {number} times.
		 * @param number
		 * @constructor
		 */
		EnrollHostFinger: async (number) => await spawnSync(SDKFile, ['enroll', `${number}`], options),
	};
};


/**
 * Does the check for a finger press on the device.
 * @param delay
 * @returns {Promise<boolean>}
 */
async function checkFingerPress(delay = 300) {
	let { stdout: fingerPressedStatus } = await SoftcomFingerPrintSDK()
	.isFingerPressed();
	return fingerPressedStatus.toString()
	.trim() === 'SUCCESS FINGER';
}

async function doEnrollmentCount(count) {
	const { stdout: EnrolStatus } = await SoftcomFingerPrintSDK()
	.EnrollHostFinger(count);

	const RESULT = EnrolStatus.toString();
	console.log(RESULT, ' RESULT FROM The enrolment');
	return RESULT.indexOf('::') !== -1 ? RESULT.split('::')[1] : '#' + RESULT.split('##')[1].toUpperCase();
}

const errorHandler = (code) => {
	console.log('Error Code: ', code);
	const ERROR_MESSAGES = [
		{
			code: '##x100C',
			message: 'BAD FINGER'
		},
		{
			code: '##x100D',
			message: 'ENROLMENT FAILURE, TRY AGAIN'
		},
		{
			code: '##x1012',
			message: 'FINGER IS NOT PRESSED'
		},
		{
			code: '##x1001',
			message: 'CAPTURE TIMEOUT, TRY AGAIN'
		}
		// TODO: Add error codes and messages here.
	];

	const ERROR_MESSAGE = ERROR_MESSAGES.find(err => err.code === code);

	return ERROR_MESSAGE ? ERROR_MESSAGE.message : 'ERROR MESSAGE NOT DEFINED';
};

/**
 * Initialize the enrollment process.
 * @param cb
 * @param killProcess
 * @returns {Promise<void>}
 */
async function initEnrollment(cb, killProcess = false) {
	// let firstRunDone = false;
	if (!killProcess) {
		// Open the device.
		let { stdout: deviceOpen } = await SoftcomFingerPrintSDK()
		.OpenDevice();

		if (deviceOpen.toString()
		.trim() === 'SUCCESS') {
			// Check if finger is pressed while telling the user to press their finger on the device.
			if (await checkFingerPress(1000)) { // There was a 1second delay that has been removed.
				const { stdout: EnrolStart } = await SoftcomFingerPrintSDK()
				.StartEnrollment();

				/**
				 * Get the unused ID from the `start`
				 * @type {string}
				 */
				const unusedId = EnrolStart.toString()
				.trim()
				.split('::')[1];
				if (unusedId !== undefined) {
					let j = 1;
					(function controlledStepLoop(i) {
						// setTimeout(function () {
						// 	if (firstRunDone && j > 1) {
						// 		cb(constructMessage('PLACE FINGER'));
						// 	}
						// }, 1000);
						setTimeout(async function () {
							let result = await doEnrollmentCount(j);
							if (!isNaN(result) && j !== 3) { // TODO: Put error cases here to be handled.
								// cb(constructMessage('REMOVE FINGER'));
								j++;
							} else if (!isNaN(result) && j === 3) {
								/**
								 * Get file and send to ble for data-beaver.
								 */
								return processEnrolledTemplate(cb);
								// return cb(constructMessage('PROCESS FINISHED'));
							} else {
								//TODO:: Remove the negative values of our error code.
								// Remember to send the negative values.
								const ERROR = errorHandler(result); // here the result is our error code.
								return cb(constructMessage(ERROR));
							}
							if (--i) controlledStepLoop(i);
						}, 250);
					})(3);
				}
			} else {
				return cb(constructMessage('Finger is not pressed'));
			}
		}
		cb(constructMessage('PLACE FINGER'));
		// firstRunDone = true;
	} else {
		await SoftcomFingerPrintSDK()
		.CloseDevice();
	}
}

function FingerprintService() {
	FingerprintService.super_.call(this, {
		uuid: '23edd8d170be477db4e30fda81aa8d62',
		characteristics: [
			new FingerprintNotifyOnlyCharacteristic(),

		]
	});
}

util.inherits(FingerprintService, BlenoPrimaryService);

bleno.on('stateChange', function (state) {
	console.log('on -> stateChange: ' + state + ', address = ' + bleno.address);

	if (state === 'poweredOn') {
		bleno.startAdvertising('FingerprintPi', ['23edd8d170be477db4e30fda81aa8d62']);
	} else {
		bleno.stopAdvertising();
	}
});

// Linux only events
/////////////////////////////////////
bleno.on('accept', (clientAddress) => {
	console.log('on :-> accept, client: ' + clientAddress);
	// bleno.mtu = 500; // manual mtu change.
	bleno.updateRssi();
});

bleno.on('disconnect', (clientAddress) => {
	console.log('on :-> disconnect, client: ' + clientAddress);
});

bleno.on('rssiUpdate', (rssi) => {
	console.log('on :-> rssiUpdate: ' + rssi);
});
//////////////////////////////////////

// bleno.on('mtuChange', function (mtu) {
// 	console.log('on -> mtuChange: ' + mtu);
// 	bleno.mtu = 500; // manual mtu change.
// });

bleno.on('advertisingStart', function (error) {
	console.log('on :-> advertisingStart: ' + (error ? 'error ' + error : 'success'));

	if (!error) {
		bleno.setServices([
			new FingerprintService()
		]);
	}
});

bleno.on('advertisingStop', function () {
	console.log('on -> advertisingStop');
});

bleno.on('servicesSet', function (error) {
	console.log('on -> servicesSet: ' + (error ? 'error ' + error : 'success'));
});


//NOTIFY STARTS//
//////////////////////////////////////////////////////////
const FingerprintNotifyOnlyCharacteristic = function () {
	FingerprintNotifyOnlyCharacteristic.super_.call(this, {
		uuid: 'dbb9219f8c074f69a70b2aabde4a3675',
		properties: ['notify'],
		descriptors: [
			new BlenoDescriptor({
				uuid: '2901',
				value: 'notify'
			})
		]
	});
};

util.inherits(FingerprintNotifyOnlyCharacteristic, BlenoCharacteristic);


FingerprintNotifyOnlyCharacteristic.prototype.onSubscribe = async (maxValueSize = 500, updateCallback) => {
	console.log(maxValueSize, ' Max value Size');
	await initEnrollment(updateCallback, false);
};
FingerprintNotifyOnlyCharacteristic.prototype.onUnsubscribe = async function () {
	await initEnrollment(null, true);
};
