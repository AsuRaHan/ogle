// startup.js
// This script runs when the application starts.

function onStart() {
    ogle.log("Startup script 'startup.js' is running onStart().");

    // Clear the default world if it exists
    ogle.world.clear();

    // Create a floor
    ogle.world.createCube({
        name: "ScriptedFloor",
        position: { x: 0, y: -0.5, z: 0 },
        scale: { x: 20, y: 0.1, z: 20 }
    });

    // Create a few objects
    var cube1 = ogle.world.createCube({
        name: "ScriptedCube1",
        position: { x: -2, y: 0.5, z: 0 },
        scale: { x: 1, y: 1, z: 1 }
    });

    var cube2 = ogle.world.createCube({
        name: "ScriptedCube2",
        position: { x: 2, y: 1.5, z: -1 },
        scale: { x: 1, y: 3, z: 1 }
    });

    // Create a light
    ogle.world.createDirectionalLight({
        name: "ScriptedSun",
        rotation: { x: -60, y: 30, z: 0 },
        intensity: 1.5
    });

    ogle.log("Scene created by startup.js");
}

var time = 0;

function onUpdate(dt) {
    time += dt;
    var cube1 = ogle.world.findByName("ScriptedCube1");
    if (ogle.entity.exists(cube1)) {
        var newY = 0.5 + Math.sin(time * 2.0) * 0.5;
        ogle.entity.setPosition(cube1, -2, newY, 0);
    }
}