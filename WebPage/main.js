import * as THREE from 'three';
import WebGL from 'three/addons/capabilities/WebGL.js';
import { GLTFLoader } from 'three/addons/loaders/GLTFLoader.js';

const scene = new THREE.Scene();
const camera = new THREE.PerspectiveCamera(75, window.innerWidth / window.innerHeight, 0.1, 1000);

const renderer = new THREE.WebGLRenderer();
renderer.setClearColor(0xFFFFFF);
renderer.setSize(window.innerWidth, window.innerHeight);
document.body.appendChild(renderer.domElement);

const loader = new GLTFLoader();

loader.load('public/test.gltf', function(gltf) {
	console.log('Loaded!');
	scene.add(gltf.scene);
}, undefined, function(error) {
	console.error(error);
});

camera.position.set(0, 0, 10);
camera.lookAt(0, 0, 0);

const light = new THREE.AmbientLight(0x404040);
scene.add(light);

function animate() {
	requestAnimationFrame(animate);
	
	// TODO: My GLTF file renders!  But...it's all black.  Why?
	renderer.render(scene, camera);
}

if (WebGL.isWebGLAvailable()) {

	// Initiate function or other initializations here
	animate();

} else {
	const warning = WebGL.getWebGLErrorMessage();
	document.getElementById('container').appendChild(warning);
}