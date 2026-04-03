// startup.js
// This script runs when the application starts.

function onStart() {
    ogle.log("Startup script 'startup.js' is running onStart().");

    ogle.world.clear();

    // Create a floor and add a static physics body
    var floor = ogle.world.createCube({
        name: "ScriptedFloor",
        position: { x: 0, y: -0.5, z: 0 },
        scale: { x: 20, y: 0.1, z: 20 }
    });
    ogle.physics.addBox(floor, { x: 10, y: 0.05, z: 10 }, "Static", 0);

    // Dynamic cube that falls down
    var cube1 = ogle.world.createCube({
        name: "ScriptedCube1",
        position: { x: -2, y: 5.0, z: 0 },
        scale: { x: 1, y: 1, z: 1 }
    });
    ogle.physics.addBox(cube1, { x: 0.5, y: 0.5, z: 0.5 }, "Dynamic", 1.0);

    // Dynamic box that collides with the floor
    var cube2 = ogle.world.createCube({
        name: "ScriptedCube2",
        position: { x: 2, y: 6.0, z: 0 },
        scale: { x: 0.8, y: 0.8, z: 0.8 }
    });
    ogle.physics.addBox(cube2, { x: 0.4, y: 0.4, z: 0.4 }, "Dynamic", 1.0);

    ogle.physics.setCollisionCallback(function (a, b) {
        ogle.log("Collision callback triggered between " + a + " and " + b);
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
        var newY = 1.5 + Math.sin(time * 2.0) * 1.5;
        ogle.entity.setPosition(cube1, -2, newY, 0);
    }
}