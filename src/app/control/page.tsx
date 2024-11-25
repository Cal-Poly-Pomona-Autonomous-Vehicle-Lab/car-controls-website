'use client'
import { useEffect, useRef, useState } from "react";
import Link from 'next/link'
import { redirect } from 'next/navigation';
import { ChevronCompactDown, ChevronCompactLeft, ChevronCompactRight, ChevronBarUp } from 'react-bootstrap-icons';
import {io, Socket} from 'socket.io-client';
import "./controlPage.css";
import { useFormState } from 'react-dom';

export default function carControls() {
    let socket = useRef<Socket | null> (null); 
    const [isData, setData] = useState(null);


    const [isLeft, setLeft] = useState(false);
    const [isRight, setRight] = useState(false);
    const [isForward, setForward] = useState(false);
    const [isBackward, setBackward] = useState(false);

    socket.current = io("http://10.110.194.54:5002", {
        transports: ["websocket"], 
    });

    useEffect(() => {
        socket.current!.on('connect', () => {
            console.log("Conneted");
            console.log(socket.current);
        })
        
        socket.current!.on("disconnect", () => {
            console.log(socket.current!.id); // undefined
          });

        socket.current!.on("connect_error", (err) => {
            console.log(err.message);
        
          })
    }, [])

    const sendKeyPressToRos = async(event: any) => {
        if (event.key === "w"){
            socket.current!.emit('keypress', {key: event.key, action: "press"}, () => {
                console.log("sent");
            });
       } else if (event.key === "s") {
            socket.current!.emit('keypress', {key: event.key, action: "press"}, () => {
                console.log("sent");
            });
       } else if (event.key === "a") {
            socket.current!.emit('keypress', {key: event.key, action: "press"}, () => {
                console.log("sent");
            });
       } else if (event.key === "d"){
            socket.current!.emit('keypress', {key: event.key, action: "press"}, () => {
                console.log("sent");
            });
       }
    }

    return (
        <>
            <div className="allStreamControls" onKeyDown={sendKeyPressToRos} tabIndex={0}> 
                <div className="videoStream" tabIndex={0}> 
                    <img className="videoFrame" src="http://10.110.194.54:5002/camera/stream" /> 
                </div>
                <div className="controls" tabIndex={0}>
                    <ChevronCompactDown size={20}></ChevronCompactDown>
                </div>
            </div>
        </>
    );
}
