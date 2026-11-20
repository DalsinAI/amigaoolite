"use strict";
// Team 5's show-reel for Oolite on GFOS: launch, the docking computer flies
// back in to the Blue Danube, a short rest in the station, and round again.
this.name = "t5_demo";
this.author = "AmiCradle Team 5";
this.version = "1.0";

this.startUpComplete = function () {
	if (player.ship.equipmentStatus("EQ_DOCK_COMP") !== "EQUIPMENT_OK") {
		player.ship.awardEquipment("EQ_DOCK_COMP");
	}
	if (player.ship.docked) this.shipDockedWithStation();
};

this.shipLaunchedFromStation = function (station) {
	if (this._dock) this._dock.stop();
	// out far enough for the approach to be worth watching
	this._dock = new Timer(this, function () { player.ship.engageAutopilotToStation(station); }, 12);
};

this.shipDockedWithStation = function () {
	if (this._launch) this._launch.stop();
	this._launch = new Timer(this, function () { player.ship.launch(); }, 8);
};
