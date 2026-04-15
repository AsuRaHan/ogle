// Demo script to verify the scripting system

// Global variables to hold our dynamic objects
var movingCube = null;
var rotatingSphere = null;
var elapsedTime = 0;

function onStart() {
    ogle.log("Starting demo script...");

    // 1. Clear the world to start fresh
    // ogle.world.clear();
    ogle.log("World cleared.");

    // 2. Create a floor
    var floor = ogle.world.createCube({
        name: "Floor",
        position: { x: 0, y: -0.5, z: 0 },
        scale: { x: 20, y: 1, z: 20 }
    });
    ogle.log("Created floor entity: " + floor);
    
    // 3. Add physics to the floor
    ogle.physics.addBox(floor, { halfExtents: { x: 10, y: 0.5, z: 10 }, bodyType: 'Static', mass: 0 });
    ogle.log("Added static physics body to floor.");

    // 4. Create a dynamic cube
    movingCube = ogle.world.createCube({
        name: "BouncingCube",
        position: { x: -3, y: 5, z: 0 },
        scale: { x: 1, y: 1, z: 1 }
    });
    ogle.log("Created cube entity: " + movingCube);
    ogle.physics.addBox(movingCube, { halfExtents: { x: 0.5, y: 0.5, z: 0.5 }, bodyType: 'Dynamic', mass: 1 });
    ogle.log("Added dynamic physics body to cube.");

    // 5. Create a dynamic sphere
    rotatingSphere = ogle.world.createSphere({
        name: "BouncingSphere",
        position: { x: 3, y: 8, z: 2 },
        radius: 0.75
    });
    ogle.log("Created sphere entity: " + rotatingSphere);
    ogle.physics.addBox(rotatingSphere, { halfExtents: { x: 0.75, y: 0.75, z: 0.75 }, bodyType: 'Dynamic', mass: 2 });
    ogle.log("Added dynamic physics body to sphere.");

    // 6. Create a light source
    ogle.world.createDirectionalLight({
        name: "Sun",
        rotation: { x: -45, y: -45, z: 0 },
        intensity: 1.5
    });
    ogle.log("Created directional light.");
    
    ogle.world.createPointLight({
        name: "BouncyLight",
        position: { x: 0, y: 3, z: 3},
        intensity: 5.0,
        color: {x: 1.0, y: 0.5, z: 0.0}
    });
    ogle.log("Created point light.");

    ogle.log("Demo script 'onStart' finished successfully.");
}

function onUpdate(dt) {
    elapsedTime += dt;

    // Make the cube move in a circle
    if (movingCube && ogle.entity.exists(movingCube)) {
        var cubePos = ogle.entity.getPosition(movingCube);
        // This won't work as expected with physics enabled, as physics will override the position.
        // To move a physics object, you should apply forces or set velocities.
        // Since that API is not exposed, we will just rotate the static sphere.
    }

    // Make the sphere rotate
    if (rotatingSphere && ogle.entity.exists(rotatingSphere)) {
        ogle.entity.setRotation(rotatingSphere, {
            x: elapsedTime * 50,
            y: 0,
            z: elapsedTime * 30
        });
    }
}

function onCollision(entityA, entityB) {
    var nameA = ogle.entity.getName(entityA);
    var nameB = ogle.entity.getName(entityB);
    ogle.log("Collision detected between '" + nameA + "' and '" + nameB + "'");
}
