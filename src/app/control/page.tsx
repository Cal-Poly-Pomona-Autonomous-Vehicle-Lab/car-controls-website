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
    const black_image = "data:image/jpeg;base64,iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAQAAAC1HAwCAAAAC0lEQVR42mNgYAAAAAMAASsJTYQAAAAASUVORK5CYII=";

    const [isData, setData] = useState(null);
    const [isLeft, setLeft] = useState(false);
    const [isRight, setRight] = useState(false);
    const [isForward, setForward] = useState(false);
    const [isBackward, setBackward] = useState(false);
    const [isStream, setStream] = useState(false);
    const [isFrame, setFrame] = useState(black_image);

    socket.current = new WebSocket("http://:18080/");

    useEffect(() => {
       isStreamLive(); 
    }, [isStream]);

    useEffect(() => {
        if (socket.current === null) {
            return; 
        }

        socket.current.onopen = (e) => {
            console.log("Opening"); 
        }

        socket.current.onclose = (e) => {
            console.log("Closing"); 
        }

        socket.current.onmessage = (mes) => {
            if (typeof mes != 'string')
                return black_image; 
            
            const base64_img = btoa(mes);  
            return base64_img;  
        }

        socket.current.onerror = (err) => {
            console.log("Error: " + err); 
        }

    }, [])

    const sendKeyPressToRos = async(event: any) => {
        if (socket.current === null) {
            return 
        }

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
        try {
            const req = await fetch("http://:5002/camera/stream");
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
        }

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
