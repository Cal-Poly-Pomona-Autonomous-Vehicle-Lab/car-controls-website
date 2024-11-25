'use client'
import { useEffect, useRef, useState } from "react";
import Link from 'next/link'
import { redirect } from 'next/navigation';
import { ChevronCompactDown, ChevronCompactLeft, ChevronCompactRight, ChevronBarUp } from 'react-bootstrap-icons';
import {io, Socket} from 'socket.io-client';
import "./controlPage.css";
import { useFormState } from 'react-dom';
import { read } from "fs";

export default function carControls() {
    let socket = useRef<Socket | null> (null); 
    const [isData, setData] = useState(null);


    const [isLeft, setLeft] = useState(false);
    const [isRight, setRight] = useState(false);
    const [isForward, setForward] = useState(false);
    const [isBackward, setBackward] = useState(false);
    const [isStream, setStream] = useState(false);

    socket.current = io("http://10.110.194.54:5002", {
        timeout: 5000, 
        transports: ["websocket"], 
    });

    useEffect(() => {
       isStreamLive(); 
    }, [isStream]);

    useEffect(() => {
        if (socket.current === null) {
            return; 
        }

        socket.current.on('connect', () => {
            console.log("Conneted");
        })

        socket.current.on("disconnect", () => {
            console.log("Disconected");
        });

        socket.current.on("connect_error", (err) => {
            console.log(err.message);
        
         })
    }, [])

    const sendKeyPressToRos = async(event: any) => {
        if (socket.current === null) {
            return 
        }

        if (event.key === "w"){
            socket.current.emit('keypress', {key: event.key, action: "press"}, () => {
                console.log("sent");
            });
       } else if (event.key === "s") {
            socket.current.emit('keypress', {key: event.key, action: "press"}, () => {
                console.log("sent");
            });
       } else if (event.key === "a") {
            socket.current.emit('keypress', {key: event.key, action: "press"}, () => {
                console.log("sent");
            });
       } else if (event.key === "d"){
            socket.current.emit('keypress', {key: event.key, action: "press"}, () => {
                console.log("sent");
            });
       }
    }

    const isStreamLive = async() => {
        try {
            const req = await fetch("http://127.0.0.1:5000/");
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
                    <img className="videoFrame" src={isStream ? "http://10.110.194.54:5002/camera/stream": "data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAQAAAC1HAwCAAAAC0lEQVR42mNgYAAAAAMAASsJTYQAAAAASUVORK5CYII="} /> 
                </div>
                <div className="controls" tabIndex={0}>
                    <ChevronCompactDown size={20}></ChevronCompactDown>
                </div>
            </div>
        </>
    );
}
