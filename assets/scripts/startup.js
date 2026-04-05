// Полноценный демонстрационный скрипт Dukglue.
// Показаны основные API-узлы: world, entity, material, light, physics, animation и пользовательские типы.

var elapsedTime = 0;
var movingCube = null;
var rotatingSphere = null;
var demoLight = null;
var importedTable = null;
var collisionCount = 0;

function createColor(name, r, g, b) {
    return { x: r, y: g, z: b };
}

function onStart() {
    ogle.log("[JS] Начало сценария: демонстрация всех возможностей движка");

    // Подготовка мира: очистка и базовый пол.
    ogle.world.clear();

    var floor = ogle.world.createCube({
        name: "Floor",
        position: { x: 0, y: -0.5, z: 0 },
        scale: { x: 50, y: 1, z: 50 }
    });
    ogle.material.setBaseColor(floor, createColor("floor", 0.15, 0.18, 0.22));
    ogle.material.setRoughness(floor, 0.95);
    ogle.physics.addBox(floor, { x: 25, y: 0.5, z: 25 }, "Static", 0);

    // Добавление моделей и объектов мира.
    importedTable = ogle.world.createModel({
        path: "assets/Table-1-FBX/Table_1_fbx.FBX",
        name: "DemoTable"
    });
    if (ogle.entity.exists(importedTable)) {
        ogle.entity.setPosition(importedTable, { x: 0, y: 0, z: 5 });
        ogle.entity.setScale(importedTable, { x: 0.4, y: 0.4, z: 0.4 });
        ogle.entity.setName(importedTable, "TableModel");
        ogle.material.setEmissiveColor(importedTable, createColor("tableEmissive", 0.05, 0.08, 0.12));
        ogle.material.setUvTiling(importedTable, { x: 1.5, y: 1.5 });
        ogle.physics.addBox(importedTable, { x: 3.0, y: 1.0, z: 3.0 }, "Static", 0);
    }

    // Свет: солнце и динамический источник.
    ogle.world.createDirectionalLight({
        name: "Sun",
        rotation: { x: -35, y: -45, z: 0 },
        color: createColor("sun", 1.0, 0.95, 0.87),
        intensity: 1.7,
        castShadows: true,
        primary: true
    });

    demoLight = ogle.world.createPointLight({
        name: "DemoLight",
        position: { x: 6, y: 4, z: -6 },
        color: createColor("purpleGlow", 0.8, 0.6, 1.0),
        intensity: 3.0,
        range: 20.0
    });
    ogle.light.setRange(demoLight, 25.0);
    ogle.light.setCastShadows(demoLight, true);
    ogle.light.setPrimary(demoLight, false);

    // Создание динамических игровых объектов.
    movingCube = ogle.world.createCube({
        name: "MovingCube",
        position: { x: -7, y: 2.0, z: 0 },
        scale: { x: 1.2, y: 1.2, z: 1.2 }
    });
    ogle.material.setBaseColor(movingCube, createColor("waterBlue", 0.2, 0.7, 1.0));
    ogle.material.setRoughness(movingCube, 0.2);
    ogle.physics.addBox(movingCube, { x: 0.6, y: 0.6, z: 0.6 }, "Dynamic", 1.0);

    rotatingSphere = ogle.world.createSphere({
        name: "SpinningSphere",
        position: { x: 7, y: 3.0, z: -3 },
        radius: 1.0
    });
    ogle.material.setBaseColor(rotatingSphere, createColor("sunset", 1.0, 0.75, 0.2));
    ogle.material.setEmissiveColor(rotatingSphere, createColor("glow", 0.2, 0.08, 0.02));
    ogle.material.setMetallic(rotatingSphere, 0.3);
    ogle.material.setReflectivity(rotatingSphere, 0.4);
    ogle.physics.addBox(rotatingSphere, { x: 1.0, y: 1.0, z: 1.0 }, "Dynamic", 2.5);

    // Пользовательский объект и методы класса Player.
    var hero = new ogle.Player(120);
    ogle.log("Player создан с HP = " + hero.getHP());
    hero.takeDamage(25);
    ogle.log("После урона HP = " + hero.getHP());

    // Показать информацию о сущностях через world / entity API.
    var allEntities = ogle.world.getEntities();
    ogle.log("Всего сущностей в мире: " + ogle.world.count());
    ogle.log("Первый ID сущности: " + (allEntities.length ? allEntities[0] : "нет сущностей"));

    var lights = ogle.world.getEntitiesByKind("Light");
    ogle.log("Найдено источников света: " + lights.length);

    var foundTable = ogle.world.findByName("TableModel");
    if (ogle.entity.exists(foundTable)) {
        ogle.log("Найдено по имени: " + ogle.entity.getName(foundTable));
    }

    // Установим callback для коллизий и сохраним текущее состояние мира.
    ogle.physics.setCollisionCallback(function (entityA, entityB) {
        if (!ogle.entity.exists(entityA) || !ogle.entity.exists(entityB)) {
            return;
        }
        var nameA = ogle.entity.getName(entityA);
        var nameB = ogle.entity.getName(entityB);
        collisionCount += 1;
        ogle.log("Коллизия " + collisionCount + ": " + nameA + " ↔ " + nameB);
    });

    ogle.world.save("assets/worlds/script_demo_world.json");

    ogle.log("Сценарий запущен. Полный набор API доступен через ogle.world, ogle.entity, ogle.material, ogle.light, ogle.physics и ogle.animation.");
}

function onUpdate(dt) {
    elapsedTime += dt;

    if (ogle.entity.exists(movingCube)) {
        var position = {
            x: -7 + Math.sin(elapsedTime) * 4.5,
            y: 2.0 + Math.sin(elapsedTime * 1.5) * 0.25,
            z: Math.cos(elapsedTime) * 2.0
        };
        ogle.entity.setPosition(movingCube, position);
    }

    if (ogle.entity.exists(rotatingSphere)) {
        var rotation = {
            x: 0,
            y: elapsedTime * 40.0,
            z: 20.0 * Math.sin(elapsedTime * 0.4)
        };
        ogle.entity.setRotation(rotatingSphere, rotation);
        var scaleFactor = 1.0 + Math.sin(elapsedTime * 2.0) * 0.15;
        ogle.entity.setScale(rotatingSphere, { x: scaleFactor, y: scaleFactor, z: scaleFactor });
    }

    if (ogle.entity.exists(demoLight)) {
        ogle.light.setColor(demoLight, createColor("pulse", 0.5 + Math.sin(elapsedTime) * 0.5, 0.4, 1.0));
        ogle.light.setIntensity(demoLight, 2.0 + Math.sin(elapsedTime * 3.5) * 1.2);
    }

    if (elapsedTime > 15.0 && ogle.entity.exists(importedTable)) {
        ogle.entity.setVisible(importedTable, false);
        ogle.log("Модель таблицы скрыта через 15 секунд");
        importedTable = null;
    }
}
