// startup.js более старый синтаксис (ES5).!!!!!
// An advanced script to demonstrate various engine features.

// Глобальные переменные для хранения ID сущностей, которые будут анимироваться
var elapsedTime = 0;
var floatingCube;
var spinningSphere;
var flickeringLight;

function onStart() {
    ogle.log("Advanced startup script 'startup.js' is running onStart().");

    // 1. Очищаем мир для создания новой сцены
    ogle.world.clear();

    // 2. Создаём окружение
    // Большой пол со статическим физическим телом
    var floor = ogle.world.createCube({
        name: "Floor",
        position: { x: 0, y: -0.5, z: 0 },
        scale: { x: 50, y: 1, z: 50 }
    });
    ogle.material.setBaseColor(floor, { x: 0.2, y: 0.2, z: 0.25 });
    ogle.material.setRoughness(floor, 0.9); // Делаем пол матовым, не блестящим
    ogle.physics.addBox(floor, { x: 25, y: 0.5, z: 25 }, "Static", 0);

    // Основной направленный источник света (солнце)
    ogle.world.createDirectionalLight({
        name: "Sun",
        rotation: { x: -45, y: -30, z: 0 },
        color: { x: 1.0, y: 0.95, z: 0.85 },
        intensity: 1.8,
        castShadows: true,
        primary: true
    });

    // 3. Создаём башню из динамических кубов для демонстрации физики
    var stackHeight = 7;
    var boxSize = 1.0;
    for (var i = 0; i < stackHeight; i++) {
        var cube = ogle.world.createCube({
            name: "StackCube_" + i,
            position: { x: 0, y: boxSize * i + boxSize / 2, z: 5 },
            scale: { x: boxSize, y: boxSize, z: boxSize }
        });
        ogle.material.setBaseColor(cube, { x: 0.8, y: 0.1 + i * 0.1, z: 0.2 });
        ogle.physics.addBox(cube, { x: boxSize / 2, y: boxSize / 2, z: boxSize / 2 }, "Dynamic", 1.0);
    }

    // 4. Создаём тяжёлый шар, который упадёт на башню
    var cannonball = ogle.world.createSphere({
        name: "Cannonball",
        position: { x: 0.5, y: 12, z: 5.5 },
        radius: 0.7
    });
    ogle.material.setBaseColor(cannonball, { x: 0.1, y: 0.1, z: 0.1 });
    ogle.material.setMetallic(cannonball, 0.9); // Металлический материал
    ogle.material.setRoughness(cannonball, 0.2); // Гладкий, блестящий
    // Используем кубический коллайдер, так как API для сферического коллайдера не документирован
    ogle.physics.addBox(cannonball, { x: 0.7, y: 0.7, z: 0.7 }, "Dynamic", 25.0); // Очень тяжёлый!

    // 5. Создаём специальные анимированные объекты
    // Парящий куб (анимация через скрипт, без физики)
    floatingCube = ogle.world.createCube({
        name: "FloatingCube",
        position: { x: -8, y: 2, z: -8 },
        scale: { x: 1, y: 1, z: 1 }
    });
    ogle.material.setBaseColor(floatingCube, { x: 0.2, y: 0.8, z: 1.0 });
    ogle.material.setEmissiveColor(floatingCube, { x: 0.1, y: 0.4, z: 0.5 }); // Добавляем свечение

    // Вращающаяся сфера
    spinningSphere = ogle.world.createSphere({
        name: "SpinningSphere",
        position: { x: 8, y: 2, z: -8 },
        radius: 1.2
    });
    ogle.material.setBaseColor(spinningSphere, { x: 1.0, y: 0.8, z: 0.2 });
    ogle.material.setReflectivity(spinningSphere, 0.5);

    // 6. Создаём мерцающий точечный источник света
    flickeringLight = ogle.world.createPointLight({
        name: "FlickerLight",
        position: { x: 8, y: 4, z: 8 },
        color: { x: 1.0, y: 0.5, z: 0.1 }, // Тёплый оранжевый цвет
        intensity: 3.0,
        range: 15.0
    });

    // 7. Устанавливаем коллбэк для отслеживания столкновений
    ogle.physics.setCollisionCallback(function (entityA, entityB) {
        var nameA = ogle.entity.getName(entityA);
        var nameB = ogle.entity.getName(entityB);
        // Избегаем спама в лог при столкновениях с полом
        if (nameA !== "Floor" && nameB !== "Floor") {
            ogle.log("Collision detected between '" + nameA + "' and '" + nameB + "'");
        }
    });

    ogle.log("Advanced scene created successfully.");
}

function onUpdate(dt) {
    elapsedTime += dt;

    // Анимация парящего куба
    if (ogle.entity.exists(floatingCube)) {
        var floatingPos = { x: -8, y: 2.0 + Math.sin(elapsedTime) * 1.5, z: -8 };
        ogle.entity.setPosition(floatingCube, floatingPos);
    }

    // Анимация вращающейся сферы
    if (ogle.entity.exists(spinningSphere)) {
        var rotation = { x: 0, y: elapsedTime * 50.0, z: 20.0 * Math.sin(elapsedTime * 0.5) };
        ogle.entity.setRotation(spinningSphere, rotation);
    }

    // Анимация мерцающего света
    if (ogle.entity.exists(flickeringLight)) {
        var intensity = 2.0 + Math.sin(elapsedTime * 10.0) * 1.5 + (Math.random() - 0.5) * 0.5;
        ogle.light.setIntensity(flickeringLight, Math.max(0, intensity));
    }
}