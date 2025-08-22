'use client'
import { useEffect, useRef, useState } from "react";
import Link from 'next/link'
import { redirect } from 'next/navigation';
import { ChevronCompactDown, ChevronCompactLeft, ChevronCompactRight, ChevronBarUp } from 'react-bootstrap-icons';
import "./controlPage.css";
import { useFormState } from 'react-dom';
import { read } from "fs";

export default function carControls() {
    let socket = useRef<WebSocket | null> (null); 
    const black_image = `data:image/jpeg;base64,
        iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAQAAAC1HAwCAAAAC0lEQVR42mNgYAAAAAMAASsJTYQAAAAASUVORK5CYII=`;

    const [isData, setData] = useState(null);
    const [isLeft, setLeft] = useState(false);
    const [isRight, setRight] = useState(false);
    const [isForward, setForward] = useState(false);
    const [isBackward, setBackward] = useState(false);
    const [isStream, setStream] = useState(false);
    const [isFrame, setFrame] = useState(black_image);

    const ipv4 = (typeof process.env.LOCALHOST === 'undefined') ? "0.0.0.0" : process.env.LOCALHOST;  


    useEffect(() => {
       isStreamLive(); 
    }, [isStream]);

    useEffect(() => {
        socket.current = new WebSocket(`ws://${ipv4}:5002/`);

        const reader = new FileReader(); 
        console.log("Initializing Websockets");

        if (socket.current === null) {
            return; 
        }

        socket.current.onopen = (e) => {
            console.log("Opening"); 
        }


        socket.current.onclose = (e) => {
            console.log("Closing"); 

            e.stopImmediatePropagation(); 
            socket.current?.close(); 
        }

        socket.current.onmessage = async(e) => {
            console.log("Recieved Message");
            console.log(e);

            console.log(socket.current?.OPEN);

            const binary_img = e.data; 
            if (binary_img.size === 0) {
                console.log("Empty Data"); 
                return;  
            } 

            reader.onloadend = () => {
                if (reader.result == null || reader.result instanceof ArrayBuffer) {
                    console.log("Failed to obtain image"); 
                    return; 
                } 

                console.log("inserting photo");

                setFrame(reader.result);
            }
            reader.readAsDataURL(binary_img); 
        }


        socket.current.onerror = (err) => {
            if (socket.current == null)
                return; 

            err.stopImmediatePropagation(); 
            console.log("Error: " + err.type);

            socket.current.close(); 
        }

        return () => {
            if (!socket.current) 
                return
            if (socket.current?.readyState !== 3) {
                socket.current?.close(); 
            }
        }
    }, [])

    const sendKeyPressToRos = async(event: any) => {
        if (socket.current === null) {
            return 
        } 

        // if (socket.current.CONNECTING === 0) {
        //     console.log("Connecting..."); 
        //     return; 
        // } 

        if (event.key === "w"){
            socket.current.send(`\{key: ${event.key}, action: \"press\"`);  
            console.log("sent forward command"); 
       } else if (event.key === "s") {
            socket.current.send(`\{key: ${event.key}, action: \"press\"`);  
            console.log("sent back command"); 
       } else if (event.key === "a") {
            socket.current.send(`\{key: ${event.key}, action: \"press\"`);  
            console.log("sent left command"); 
       } else if (event.key === "d"){
            socket.current.send(`\{key: ${event.key}, action: \"press\"`);  
            console.log("sent right command"); 
       }
    }

    const isStreamLive = async() => {
        /* try {
            const req = await fetch(`http://${ipv4}:5002/camera/stream`);
            if (!req.ok) {
                return;
            }

            const reader = req.body?.getReader();

            setStream(true); 
            while (true){
                const value = await reader?.read(); 
                if (value?.done) { break }
            }

        } catch(error) {
            console.log(error);
        } finally {
            setStream(false);
        } */ 
    }

    return (
        <>
            <div className="allStreamControls" onKeyDown={sendKeyPressToRos} tabIndex={0}> 
                <div className="videoStream" tabIndex={0}> 
                    <div className="backgroundColor" /> 
                    <img className="videoFrame" src={isFrame} /> 
                </div>
                <div className="controls" tabIndex={0}>
                    <ChevronCompactDown size={20}></ChevronCompactDown>
                </div>
            </div>
        </>
    );
}
