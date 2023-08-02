import * as THREE from "/node_modules/.vite/deps/three.js?v=24ff2ca1";
import WebGL from "/node_modules/.vite/deps/three_addons_capabilities_WebGL__js.js?v=244a07e3";
import { GLTFLoader } from "/node_modules/.vite/deps/three_addons_loaders_GLTFLoader__js.js?v=11ed29e3";

let scene = new THREE.Scene();
let camera = new THREE.PerspectiveCamera(75, window.innerWidth / window.innerHeight, 0.1, 1000);
let renderer = new THREE.WebGLRenderer();
let spotLight = new THREE.SpotLight(0xffffff);
let clock = new THREE.Clock(true);

// TODO: Lighting is terrible.  Can we make it any better?

renderer.setClearColor(0x000000);
renderer.setSize(window.innerWidth, window.innerHeight);

spotLight.angle = Math.PI / 3.0;
spotLight.penumbra = 0.5;
scene.add(spotLight);

document.body.appendChild(renderer.domElement);

let loader = new GLTFLoader();

loader.load('Maze.gltf', function(data) {
	scene.add(data.scene);
}, undefined, function(error) {
	console.error(error);
});

let mazeData = undefined;
let eps = 0.0001;

async function GetMazeData() {
    let response = await fetch("Maze.json");
    mazeData = await response.json();
    for(let i = 0; i < mazeData.length; i++) {
        let node = mazeData[i];
        node.location = new THREE.Vector3(node.location.x, node.location.y, node.location.z);
    }
    console.log(mazeData);
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
    
    tick(deltaTime) {
        if(this.state === "initialize") {
            this.node = mazeData[0];
            this.position.copy(this.node.location);
            this.state = "choose_next_node";
        } else if(this.state === "choose_next_node") {
            // TODO: Choose a node at random that we have not yet visited.  Once we have visited all nodes, reset.
            let i = Math.floor(Math.random() * this.node.connections.length);
            if(i === this.node.connections.length)
                i = this.node.connections.length - 1;
            this.nextNode = mazeData[this.node.connections[i]];
            this.state = "acquire_heading";
        } else if(this.state === "acquire_heading") {
            let targetDirection = new THREE.Vector3();
            targetDirection.subVectors(this.nextNode.location, this.node.location);
            targetDirection.normalize();
            if(this.direction.angleTo(targetDirection) >= Math.PI - eps) {
                this.direction.x += 0.1;
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
                this.position.copy(this.nextNode.location);
                this.node = this.nextNode;
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
    pilot.tick(deltaTime);
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