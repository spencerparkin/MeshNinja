import * as THREE from "/node_modules/.vite/deps/three.js?v=24ff2ca1";
import WebGL from "/node_modules/.vite/deps/three_addons_capabilities_WebGL__js.js?v=244a07e3";
import { GLTFLoader } from "/node_modules/.vite/deps/three_addons_loaders_GLTFLoader__js.js?v=11ed29e3";

const scene = new THREE.Scene();
const camera = new THREE.PerspectiveCamera(75, window.innerWidth / window.innerHeight, 0.1, 1000);

const renderer = new THREE.WebGLRenderer();
renderer.setClearColor(0x000000);
renderer.setSize(window.innerWidth, window.innerHeight);
document.body.appendChild(renderer.domElement);

const loader = new GLTFLoader();
let mesh = undefined;

loader.load('test.gltf', function(gltf) {
	console.log('Loaded!');
	scene.add(gltf.scene);
	mesh = gltf.scene.children[0];
}, undefined, function(error) {
	console.error(error);
});

camera.position.set(0, 0, 20);
camera.lookAt(0, 0, 0);

//const light = new THREE.AmbientLight(0xFFFFFF, 1.0);
//scene.add(light);

const directionalLight = new THREE.DirectionalLight( 0xffffff, 1.0 );
directionalLight.position.set(1.0, 1.0, 1.0);
scene.add( directionalLight );

function animate() {
	requestAnimationFrame(animate);
	
    if(mesh !== undefined) {
		mesh.rotation.x += 0.01;
		mesh.rotation.y += 0.02;
	}
    
	renderer.render(scene, camera);
}

if (WebGL.isWebGLAvailable()) {

	// Initiate function or other initializations here
	animate();

} else {
	const warning = WebGL.getWebGLErrorMessage();
	document.getElementById('container').appendChild(warning);
}