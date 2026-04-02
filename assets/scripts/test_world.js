var animatedCube = -1;
var elapsedTime = 0.0;
var lamp = -1;

function onStart() {
    ogle.log("test_world.js loaded");

    animatedCube = ogle.world.createCube({
        name: "ScriptedCube",
        position: { x: -2.5, y: 0.5, z: -2.5 },
        scale: { x: 0.9, y: 0.9, z: 0.9 }
    });

    ogle.world.createCube({
        name: "ScriptedTower",
        position: { x: -4.0, y: 1.4, z: -3.5 },
        scale: { x: 0.7, y: 2.8, z: 0.7 }
    });

    ogle.world.createCube({
        name: "ScriptedMarker",
        position: { x: 2.7, y: -0.3, z: -2.2 },
        scale: { x: 0.5, y: 0.5, z: 0.5 }
    });

    lamp = ogle.world.createPointLight({
        name: "ScriptLamp",
        position: { x: -2.0, y: 2.5, z: -2.0 },
        color: { x: 1.0, y: 0.7, z: 0.35 },
        intensity: 2.5,
        range: 6.0
    });

    // The JS API works with scene-style namespaces: world, entity, material and light.
    ogle.entity.setRotation(animatedCube, { x: 0.0, y: 30.0, z: 0.0 });
    ogle.material.setBaseColor(animatedCube, { x: 0.9, y: 0.95, z: 1.0 });
}

function onUpdate(dt) {
    elapsedTime += dt;

    if (ogle.entity.exists(animatedCube)) {
        ogle.entity.setPosition(animatedCube, {
            x: -2.5,
            y: 0.6 + Math.sin(elapsedTime * 2.0) * 0.4,
            z: -2.5
        });
        ogle.entity.setRotation(animatedCube, 0.0, elapsedTime * 90.0, 0.0);
    }

    if (ogle.entity.exists(lamp)) {
        ogle.light.setIntensity(lamp, 2.0 + Math.sin(elapsedTime * 3.0) * 0.5);
    }
}
