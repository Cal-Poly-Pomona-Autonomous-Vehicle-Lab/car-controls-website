import {exec} from 'child_process'; 

async function run_command(command: string) {
    return new Promise((resolve, reject) => {
        exec(command, (error, stdout, stderr) => {
            if (error) {
                reject(`Error executing command: ${error.message}`);
                return;
            }

            if (stderr) {
                reject(`Command execution resulted in an error ${stderr}`);
                return; 
            }

            resolve(stdout);
        });
    });
}

async function init_vpn() {
    try {
        const command = await run_command("openvpn"); 
        console.log(command);
    } catch(error) {
        console.error("Error:\n", error); 
    }
}

init_vpn()
