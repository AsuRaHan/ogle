(function () {
    var native = this.__ogleNative;

    function unpackVec3(a, b, c) {
        if (typeof a === 'object' && a !== null) {
            return [Number(a.x) || 0, Number(a.y) || 0, Number(a.z) || 0];
        }
        return [Number(a) || 0, Number(b) || 0, Number(c) || 0];
    }

    var ogle = {
        log: native.log,
        world: {
            clear: native.clearWorld,
            save: native.saveWorld,
            load: native.loadWorld,
            count: native.getEntityCount,
            getEntities: native.getAllEntities,
            getEntitiesByKind: native.getEntitiesByKind,
            findByName: native.findEntityByName,
            createEmpty: function (name) { return native.createEmpty(name || 'Entity'); },
            createCube: function (options) {
                options = options || {};
                var p = unpackVec3(options.position || { x: 0, y: 0, z: 0 });
                var s = unpackVec3(options.scale || { x: 1, y: 1, z: 1 });
                return native.createCube(options.name || 'Cube', p[0], p[1], p[2], s[0], s[1], s[2], options.texture || '');
            },
            createModel: function (options) {
                options = options || {};
                return native.createModel(options.path || '', options.name || 'Model');
            },
            createDirectionalLight: function (options) {
                options = options || {};
                var r = unpackVec3(options.rotation || { x: -50, y: 45, z: 0 });
                var c = unpackVec3(options.color || { x: 1, y: 1, z: 1 });
                return native.createDirectionalLight(options.name || 'DirectionalLight', r[0], r[1], r[2], c[0], c[1], c[2], Number(options.intensity == null ? 1 : options.intensity), options.castShadows !== false, options.primary !== false);
            },
            createPointLight: function (options) {
                options = options || {};
                var p = unpackVec3(options.position || { x: 0, y: 1.5, z: 0 });
                var c = unpackVec3(options.color || { x: 1, y: 1, z: 1 });
                return native.createPointLight(options.name || 'PointLight', p[0], p[1], p[2], c[0], c[1], c[2], Number(options.intensity == null ? 2 : options.intensity), Number(options.range == null ? 8 : options.range));
            },
            destroyEntity: native.destroyEntity
        },
        entity: {
            exists: native.entityExists,
            destroy: native.destroyEntity,
            getName: native.getName,
            setName: native.setName,
            getKind: native.getKind,
            getPosition: native.getPosition,
            getRotation: native.getRotation,
            getScale: native.getScale,
            getEnabled: native.getEnabled,
            setEnabled: native.setEnabled,
            getVisible: native.getVisible,
            setVisible: native.setVisible,
            setPosition: function (entity, a, b, c) { var v = unpackVec3(a, b, c); return native.setPosition(entity, v[0], v[1], v[2]); },
            setRotation: function (entity, a, b, c) { var v = unpackVec3(a, b, c); return native.setRotation(entity, v[0], v[1], v[2]); },
            setScale: function (entity, a, b, c) { var v = unpackVec3(a, b, c); return native.setScale(entity, v[0], v[1], v[2]); }
        },
        material: {
            getTexture: native.getTexture,
            setTexture: native.setTexture,
            getShaderProgram: native.getShaderProgram,
            setShaderProgram: native.setShaderProgram,
            createAsset: native.createMaterialAsset,
            applyAsset: native.applyMaterialAsset,
            saveLibrary: native.saveMaterialLibrary,
            loadLibrary: native.loadMaterialLibrary,
            getBaseColor: native.getMaterialBaseColor,
            setBaseColor: function (entity, a, b, c) { var v = unpackVec3(a, b, c); return native.setMaterialBaseColor(entity, v[0], v[1], v[2]); },
            getEmissiveColor: native.getMaterialEmissiveColor,
            setEmissiveColor: function (entity, a, b, c) { var v = unpackVec3(a, b, c); return native.setMaterialEmissiveColor(entity, v[0], v[1], v[2]); },
            getUvTiling: native.getMaterialUvTiling,
            setUvTiling: function (entity, x, y) {
                if (typeof x === 'object' && x !== null) {
                    return native.setMaterialUvTiling(entity, Number(x.x) || 0, Number(x.y) || 0);
                }
                return native.setMaterialUvTiling(entity, Number(x) || 0, Number(y) || 0);
            },
            getUvOffset: native.getMaterialUvOffset,
            setUvOffset: function (entity, x, y) {
                if (typeof x === 'object' && x !== null) {
                    return native.setMaterialUvOffset(entity, Number(x.x) || 0, Number(x.y) || 0);
                }
                return native.setMaterialUvOffset(entity, Number(x) || 0, Number(y) || 0);
            },
            getRoughness: native.getMaterialRoughness,
            setRoughness: native.setMaterialRoughness,
            getMetallic: native.getMaterialMetallic,
            setMetallic: native.setMaterialMetallic,
            getAlphaCutoff: native.getMaterialAlphaCutoff,
            setAlphaCutoff: native.setMaterialAlphaCutoff,
            getEmissiveTexture: native.getEmissiveTexture,
            setEmissiveTexture: native.setEmissiveTexture
        },
        animation: {
            createAsset: native.createAnimationAsset,
            applyAsset: native.applyAnimationAsset,
            saveLibrary: native.saveAnimationLibrary,
            loadLibrary: native.loadAnimationLibrary
        },
        light: {
            getColor: native.getLightColor,
            setColor: function (entity, a, b, c) { var v = unpackVec3(a, b, c); return native.setLightColor(entity, v[0], v[1], v[2]); },
            getIntensity: native.getLightIntensity,
            setIntensity: native.setLightIntensity,
            getRange: native.getLightRange,
            setRange: native.setLightRange,
            getCastShadows: native.getLightCastShadows,
            setCastShadows: native.setLightCastShadows,
            getPrimary: native.getLightPrimary,
            setPrimary: native.setLightPrimary,
            getType: native.getLightType,
            setType: native.setLightType
        },
        physics: {
            addBox: function (entity, halfExtents, bodyType, mass) {
                halfExtents = halfExtents || { x: 0.5, y: 0.5, z: 0.5 };
                return native.physicsAddBox(entity, halfExtents.x, halfExtents.y, halfExtents.z, bodyType || 'Dynamic', mass || 1.0);
            },
            addSphere: function (entity, radius, bodyType, mass) {
                return native.physicsAddSphere(entity, radius || 0.5, bodyType || 'Dynamic', mass || 1.0);
            },
            addCapsule: function (entity, radius, height, bodyType, mass) {
                return native.physicsAddCapsule(entity, radius || 0.5, height || 1.0, bodyType || 'Dynamic', mass || 1.0);
            },
            removeBody: native.physicsRemoveBody,
            setCollisionCallback: native.physicsSetCollisionCallback
        }
    };

    this.ogle = ogle;
})();