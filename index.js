const util = require('util');
const { spawnSync } = require('child_process');
const path = require('path');
const bleno = require('bleno');
const fs = require('fs');

const SDKFile = path.join(path.resolve(__dirname, 'SoftcomFingerPrintSDK'));

const BlenoPrimaryService = bleno.PrimaryService;
const BlenoCharacteristic = bleno.Characteristic;
const BlenoDescriptor = bleno.Descriptor;

let ACTION_TODO = 'IDENTIFY';
/**
 * Constructs a message as a buffer for reuse in the application.
 * @param message
 * @returns {Buffer}
 */
const constructMessage = message => new Buffer.from(message, 'utf8');


const processEnrolledTemplate = async cb => {
	let file = fs.readFileSync(path.resolve(path.join(__dirname, 'tpl.bin')));
	// TODO: Delete file after usage.
	
	cb(file) // raw file to the ble client side.
};

/**
 * Softcom Fingerprint SDK
 * @returns {{CloseDevice: (function(): Buffer), EnrollHostFinger: (function(*): Buffer), StartEnrollment: (function(*): Buffer), isFingerPressed: (function(*): Buffer), EnrollFinger: (function(*): Buffer), OpenDevice: (function(): Buffer), Identify: (function(): Buffer)}}
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
		 * @param delay
		 * @returns {Promise<*>}
		 */
		isFingerPressed: async (delay) => await spawnSync(SDKFile, ['finger', `${delay}`], options),
		/**
		 * Start the enrollment process and return a "SUCCESS ENROLLMENT ::ID"
		 * @returns {Promise<Buffer>}
		 * @constructor
		 */
		StartEnrollment: async (isHost) => await spawnSync(SDKFile, ['start', isHost ? 'host' : ''], options),
		/**
		 * Enroll the capture finger {number} times.
		 * @param number
		 * @constructor
		 */
		EnrollFinger: async (number) => await spawnSync(SDKFile, ['enrol', `${number}`], options),
		Identify: async () => await spawnSync(SDKFile, ['identify'], options),
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
	.isFingerPressed(delay);
	return fingerPressedStatus.toString()
	.trim() === 'SUCCESS FINGER';
}

async function doEnrollmentCount(count) {
	const { stdout: EnrolStatus } = await SoftcomFingerPrintSDK()
	.EnrollHostFinger(count);
	return EnrolStatus.toString()
	.split('::')[1];
}


/**
 * Initialize the enrollment process.
 * @param cb
 * @param killProcess
 * @returns {Promise<void>}
 */
async function initEnrollment(cb, killProcess = false) {
	let firstRunDone = false;
	if (!killProcess) {
		// Open the device.
		let { stdout: deviceOpen } = await SoftcomFingerPrintSDK()
		.OpenDevice();

		if (deviceOpen.toString()
		.trim() === 'SUCCESS') {
			// Check if finger is pressed while telling the user to press their finger on the device.
			if (await checkFingerPress(3000)) {
				setTimeout(async function () {
					const { stdout: EnrolStart } = await SoftcomFingerPrintSDK()
					.StartEnrollment(true); // isHost.

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
							setTimeout(function () {
								if (firstRunDone && j > 1) {
									cb(constructMessage('PLACE FINGER'));
								}
							}, 2000);
							setTimeout(async function () {
								let result = await doEnrollmentCount(j);
								if (!isNaN(result) && j !== 3) {
									cb(constructMessage('REMOVE FINGER'));
									j++;
								} else if (!isNaN(result) && j === 3) {
									/**
									 * Get file and send to ble for data-beaver.
									 */
									return processEnrolledTemplate(cb);
									// return cb(constructMessage('PROCESS FINISHED'));
								} else {
									return cb(constructMessage('Error, please try again.'));
								}
								if (--i) controlledStepLoop(i); //  decrement i and call myLoop again if i > 0
							}, 3000);
						})(3);
					}
				}, 2000);
			} else {
				return cb(constructMessage('Finger is not pressed'));
			}
		}
		cb(constructMessage('PLACE FINGER'));
		firstRunDone = true;
	} else {
		await SoftcomFingerPrintSDK()
		.CloseDevice();
	}
}

/**
 * Initialize the identification process.
 * @param cb
 * @param killProcess
 * @returns {Promise<void>}
 */
async function initIdentification(cb, killProcess) {
	if (!killProcess) {
		let { stdout: deviceOpen } = await SoftcomFingerPrintSDK()
		.OpenDevice();

		setTimeout(async function () {
			if (deviceOpen.toString()
			.trim() === 'SUCCESS') {
				// Check if finger is pressed while telling the user to press their finger on the device.
				if (await checkFingerPress(3000)) {
					setTimeout(async function () {
						//While the user's finger is pressed, run the identification.
						const { stdout: IdentificationStatus } = await SoftcomFingerPrintSDK()
						.Identify();

						/**
						 * Get the identified finger's ID from the sensor's database.
						 */
						const enrollmentId = String(IdentificationStatus)
						.toString()
						.trim()
						.split('::')[1];


						/**
						 * Validate ID and log user's time of entry in the database and tell them to REMOVE FINGER or anything nice.
						 */
						if (!isNaN(enrollmentId) && enrollmentId >= 0) {
							// TODO: DB Calls Here.
							console.log(enrollmentId, ' : Identified @ ', new Date(Date.now()));
							cb(constructMessage(enrollmentId + ' : Identified @ ' + new Date(Date.now()) + ' [PROCESS FINISHED]'));
						}
						// TODO: Capture more error messages here.
						else {
							cb(constructMessage('Could not identify finger. Please try again or contact administrator.'));
						}
					}, 3000);
				} else {
					return cb(constructMessage('Finger is not pressed'));
				}
			}
		}, 2000);
		cb(constructMessage('PLACE FINGER'));
	} else {
		await SoftcomFingerPrintSDK()
		.CloseDevice();
	}
}

function FingerprintService() {
	FingerprintService.super_.call(this, {
		uuid: '23edd8d170be477db4e30fda81aa8d62',
		characteristics: [
			new FingerprintReadOnlyCharacteristic(),
			new FingerprintWriteOnlyCharacteristic(),
			new FingerprintNotifyOnlyCharacteristic(),
			new FingerprintIndicateOnlyCharacteristic()
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

	bleno.updateRssi();
});

bleno.on('disconnect', (clientAddress) => {
	console.log('on :-> disconnect, client: ' + clientAddress);
});

bleno.on('rssiUpdate', (rssi) => {
	console.log('on :-> rssiUpdate: ' + rssi);
});
//////////////////////////////////////

/*  bleno.on('mtuChange', function(mtu) {
    console.log('on -> mtuChange: ' + mtu);
  });*/

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

const FingerprintReadOnlyCharacteristic = function () {
	FingerprintReadOnlyCharacteristic.super_.call(this, {
		uuid: 'be3674ee001b4d02a6410bde9e03d971',
		properties: ['read'],
		value: new Buffer('MOONSHOT DOING SOFTWORK'),
		descriptors: [
			new BlenoDescriptor({
				uuid: '2901',
				value: 'read'
			})
		]
	});
};
util.inherits(FingerprintReadOnlyCharacteristic, BlenoCharacteristic);
//READ ENDS//

//WRITE STARTS//
const FingerprintWriteOnlyCharacteristic = function () {
	FingerprintWriteOnlyCharacteristic.super_.call(this, {
		uuid: '781ea64d950f488a9682e81d2a279e47',
		properties: ['write'],
		descriptors: [
			new BlenoDescriptor({
				uuid: '2901',
				value: 'write'
			})
		]
	});
};

util.inherits(FingerprintWriteOnlyCharacteristic, BlenoCharacteristic);

FingerprintWriteOnlyCharacteristic.prototype.onWriteRequest = (data, offset = null, withoutResponse = false, callback) => {
	console.log('WriteCharacteristic write request: ' + data.toString() /*+ ' ' + offset + ' ' + withoutResponse*/);

	const ENTRY = data.toString();
	switch (ENTRY) {
		case 'A':
			ACTION_TODO = 'ENROL';
			break;
		case 'B':
			ACTION_TODO = 'IDENTIFY';
			break;
	}

	console.log('WRITE ACTION TO BE DONE: ', ACTION_TODO);
	callback(this.RESULT_SUCCESS);
};


//WRITE ENDS//

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

FingerprintNotifyOnlyCharacteristic.prototype.onSubscribe = async (maxValueSize, updateValueCallback) => {
	console.log('NotifyCharacteristic subscribe');
	switch (ACTION_TODO) {
		case 'ENROL':
			// DO enrollment;
			await initEnrollment(updateValueCallback);
			break;
		case 'IDENTIFY':
			// Do Identification.
			await initIdentification(updateValueCallback);
			break;
	}
};
FingerprintNotifyOnlyCharacteristic.prototype.onUnsubscribe = async function () {
	switch (ACTION_TODO) {
		case 'ENROL':
			await initEnrollment(null, true);
			break;
		case 'IDENTIFY':
			await initIdentification(null, true);
			break;

	}
	// ACTION_TODO = undefined;
};
////////////////////////////////////////////////////////////
//NOTIFY ENDS//

//INDICATE START//
//////////////////////////////////////////////////////////
const FingerprintIndicateOnlyCharacteristic = function () {
	FingerprintIndicateOnlyCharacteristic.super_.call(this, {
		uuid: 'a275854014e7420bb3e4d70504462b6b',
		properties: ['indicate'],
		descriptors: [
			new BlenoDescriptor({
				uuid: '2901',
				value: 'indicate'
			})
		]
	});
};

util.inherits(FingerprintIndicateOnlyCharacteristic, BlenoCharacteristic);

FingerprintIndicateOnlyCharacteristic.prototype.onSubscribe = (maxValueSize, updateValueCallback) => {
	console.log('IndicateCharacteristic subscribe');

	this.counter = 0;
	this.changeInterval = setInterval(function () {
		var data = new Buffer(4);
		data.writeUInt32LE(this.counter, 0);

		console.log('IndicateCharacteristic update value: ' + this.counter);
		updateValueCallback(data);
		this.counter++;
	}.bind(this), 2000);
};

FingerprintIndicateOnlyCharacteristic.prototype.onUnsubscribe = () => {
	console.log('IndicateCharacteristic unsubscribe');

	if (this.changeInterval) {
		clearInterval(this.changeInterval);
		this.changeInterval = null;
	}
};

FingerprintIndicateOnlyCharacteristic.prototype.onIndicate = () => {
	console.log('IndicateCharacteristic on indicate');
};
//////////////////////////////////////////////////////////
//INDICATE ENDS//
