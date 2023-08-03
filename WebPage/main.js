import * as THREE from "/node_modules/.vite/deps/three.js?v=24ff2ca1";
import WebGL from "/node_modules/.vite/deps/three_addons_capabilities_WebGL__js.js?v=244a07e3";
import { GLTFLoader } from "/node_modules/.vite/deps/three_addons_loaders_GLTFLoader__js.js?v=11ed29e3";

let scene = new THREE.Scene();
let camera = new THREE.PerspectiveCamera(50, window.innerWidth / window.innerHeight, 0.1, 1000);
let renderer = new THREE.WebGLRenderer();
let spotLight = new THREE.SpotLight(0xffffff);
let ambientLight = new THREE.AmbientLight(0x303030);
let directionalLight = new THREE.DirectionalLight(0xffffff, 1);
let clock = new THREE.Clock(true);

renderer.setClearColor(0x000000);
renderer.setSize(window.innerWidth, window.innerHeight);

spotLight.angle = Math.PI / 3.0;
spotLight.penumbra = 0.5;
scene.add(spotLight);

directionalLight.position.set(0, 1, 0);
directionalLight.castShadow = true;
scene.add(directionalLight);

scene.add(ambientLight);

document.body.appendChild(renderer.domElement);

addEventListener("resize", (event) => {
    renderer.setSize(window.innerWidth, window.innerHeight);
    camera.aspect = window.innerWidth / window.innerHeight;
    camera.updateProjectionMatrix();
});

// TODO: Textures and texture mapping?
// TODO: Add ambient sound and positional sounds?

let loader = new GLTFLoader();

loader.load('Maze.gltf', function(data) {
	scene.add(data.scene);
}, undefined, function(error) {
	console.error(error);
});

let mazeData = undefined;
let eps = 0.0001;
let masterVisitCount = 1;

async function GetMazeData() {
    let response = await fetch("Maze.json");
    mazeData = await response.json();
    for(let i = 0; i < mazeData.length; i++) {
        let node = mazeData[i];
        node.location = new THREE.Vector3(node.location.x, node.location.y, node.location.z);
        node["visitCount"] = 0;
    }
    //console.log(mazeData);
}

await GetMazeData();

function Lerp(vecA, vecB, alpha) {
    let delta = new THREE.Vector3();
    delta.subVectors(vecB, vecA);
    delta.multiplyScalar(alpha);
    let result = new THREE.Vector3();
    result.addVectors(vecA, delta);
    return result;
}

class Pilot {
    constructor() {
        this.position = new THREE.Vector3(0.0, 0.0, 0.0);
        this.direction = new THREE.Vector3(0.0, 0.0, 1.0);
        this.state = "initialize";
        this.node = undefined;
        this.nextNode = undefined;
        this.speed = 4.0;
    }
    
    tick(deltaTime, elapsedTime) {
        if(this.state === "initialize") {
            // TODO: Start in a random location.
            this.node = mazeData[0];
            this.node.visitCount = masterVisitCount++;
            this.position.copy(this.node.location);
            this.state = "choose_next_node";
        } else if(this.state === "choose_next_node") {
            let smallestCount = 9999999;
            let chosenNode = undefined;
            for(let i = 0; i < this.node.connections.length; i++) {
                let adjacentNode = mazeData[this.node.connections[i]];
                if(adjacentNode.visitCount < smallestCount) {
                    smallestCount = adjacentNode.visitCount;
                    chosenNode = adjacentNode;
                }
            }
            if(chosenNode === undefined) {
                chosenNode = mazeData[this.node.connections[0]];
            }
            this.nextNode = chosenNode;
            this.state = "acquire_heading";
        } else if(this.state === "acquire_heading") {
            let targetDirection = new THREE.Vector3();
            targetDirection.subVectors(this.nextNode.location, this.node.location);
            targetDirection.normalize();
            if(this.direction.angleTo(targetDirection) >= Math.PI - eps) {
                this.direction.x += 0.1;    // TODO: This is actually dumb.  Fix it.
            } else {
                this.direction = Lerp(this.direction, targetDirection, 0.05);
            }
            this.direction.normalize();
            if(this.direction.distanceToSquared(targetDirection) < eps)
                this.state = "acquire_target_location";
        } else if(this.state === "acquire_target_location") {
            let moveDirection = new THREE.Vector3();
            moveDirection.subVectors(this.nextNode.location, this.node.location);
            moveDirection.normalize();
            let moveDistance = this.speed * deltaTime;
            
            let oldDistanceToTarget = Math.sqrt(this.position.distanceToSquared(this.nextNode.location));
            this.position.addScaledVector(moveDirection, moveDistance);
            let newDistanceToTarget = Math.sqrt(this.position.distanceToSquared(this.nextNode.location));
            
            if(newDistanceToTarget > oldDistanceToTarget) {
                this.node = this.nextNode;
                this.node.visitCount = masterVisitCount++;
                this.state = "choose_next_node";
            }
        }
    }
    
    updateScene() {
        let targetPos = new THREE.Vector3();
        targetPos.addVectors(this.position, this.direction);
        camera.position.copy(this.position);
        camera.lookAt(targetPos.x, targetPos.y, targetPos.z);
        spotLight.position.copy(this.position);
        spotLight.target.position.copy(targetPos);
        spotLight.target.updateMatrixWorld();
    }
}

let pilot = new Pilot();

function Tick() {
    let deltaTime = clock.getDelta();
    let elapsedTime = clock.getElapsedTime();
    pilot.tick(deltaTime, elapsedTime);
    pilot.updateScene();
}

function Run() {
	requestAnimationFrame(Run);
	Tick();
	renderer.render(scene, camera);
}

if (WebGL.isWebGLAvailable()) {
	Run();
} else {
	let warning = WebGL.getWebGLErrorMessage();
	document.getElementById('container').appendChild(warning);
}