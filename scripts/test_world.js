var animatedCube = -1;
var elapsedTime = 0.0;

function onStart() {
    log("test_world.js loaded");

    animatedCube = spawnCube("ScriptedCube", -2.5, 0.5, -2.5, 0.9, 0.9, 0.9);
    spawnCube("ScriptedTower", -4.0, 1.4, -3.5, 0.7, 2.8, 0.7);
    spawnCube("ScriptedMarker", 2.7, -0.3, -2.2, 0.5, 0.5, 0.5);

    setRotation(animatedCube, 0.0, 30.0, 0.0);
}

function onUpdate(dt) {
    elapsedTime += dt;

    if (entityExists(animatedCube)) {
        setPosition(animatedCube, -2.5, 0.6 + Math.sin(elapsedTime * 2.0) * 0.4, -2.5);
        setRotation(animatedCube, 0.0, elapsedTime * 90.0, 0.0);
    }
}
