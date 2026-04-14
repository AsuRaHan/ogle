(function () {
    // C++ bindings (world, entity, physics, input, log, Player) are now global.
    // This script creates a more convenient 'ogle' object that wraps them.

    // Reference to global 'this' which holds the C++ bindings
    var global = this;

    // Helper to unpack vec3 from object or individual components
    function unpackVec3(a, b, c) {
        if (typeof a === 'object' && a !== null) {
            return [Number(a.x) || 0, Number(a.y) || 0, Number(a.z) || 0];
        }
        return [Number(a) || 0, Number(b) || 0, Number(c) || 0];
    }

    var ogle = {
        log: global.log.log.bind(global.log),

        world: {
            clear: global.world.clear.bind(global.world),
            createCube: function (options) {
                options = options || {};
                var pos = options.position ? unpackVec3(options.position) : [0, 0, 0];
                var scale = options.scale ? unpackVec3(options.scale) : [1, 1, 1];
                return global.world.createCube(options.name || 'Cube', pos, scale);
            },
            createSphere: function (options) {
                options = options || {};
                var pos = options.position ? unpackVec3(options.position) : [0, 0, 0];
                var radius = Number(options.radius) || 0.5;
                return global.world.createSphere(options.name || 'Sphere', pos, radius);
            },
            createDirectionalLight: function (options) {
                options = options || {};
                var rot = options.rotation ? unpackVec3(options.rotation) : [-50, 45, 0];
                var col = options.color ? unpackVec3(options.color) : [1, 1, 1];
                var intensity = Number(options.intensity == null ? 1.0 : options.intensity);
                var castShadows = options.castShadows !== false;
                var primary = options.primary !== false;
                return global.world.createDirectionalLight(options.name || 'DirectionalLight', rot, col, intensity, castShadows, primary);
            },
            createPointLight: function (options) {
                options = options || {};
                var pos = options.position ? unpackVec3(options.position) : [0, 1.5, 0];
                var col = options.color ? unpackVec3(options.color) : [1, 1, 1];
                var intensity = Number(options.intensity == null ? 2.0 : options.intensity);
                var range = Number(options.range == null ? 8.0 : options.range);
                return global.world.createPointLight(options.name || 'PointLight', pos, col, intensity, range);
            }
        },
        entity: {
            exists: global.entity.exists.bind(global.entity),
            getName: global.entity.getName.bind(global.entity),
            getPosition: global.entity.getPosition.bind(global.entity),
            getRotation: global.entity.getRotation.bind(global.entity),
            setPosition: function (entityId, a, b, c) {
                var v = unpackVec3(a, b, c);
                return global.entity.setPosition(entityId, v);
            },
            setRotation: function (entityId, a, b, c) {
                var v = unpackVec3(a, b, c);
                return global.entity.setRotation(entityId, v);
            }
        },

        input: global.input,

        physics: {
            addBox: function (entityId, options) {
                options = options || {};
                var halfExtents = options.halfExtents ? unpackVec3(options.halfExtents) : [0.5, 0.5, 0.5];
                var bodyType = options.bodyType || 'Dynamic';
                var mass = Number(options.mass == null ? 1.0 : options.mass);
                return global.physics.addBox(entityId, halfExtents, bodyType, mass);
            }
        },

        Player: global.Player
    };

    // Expose the new API wrapper as 'ogle' (lowercase) for scripts to use.
    global.ogle = ogle;
    
    // The original C++ binding creates a global 'OGLE' (uppercase).
    // The old_startup script uses 'ogle' (lowercase).
    // The bootstrap script originally created 'ogle'.
    // To maintain compatibility and avoid confusion, we can check if OGLE exists
    // and if it's different from our new 'ogle' object, and decide on a strategy.
    // For now, we'll just create 'ogle' as that seems to be the convention in scripts.
    // If 'OGLE' is needed, scripts can refer to it directly.
})();
