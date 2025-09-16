'use client'
import { use, useEffect, useRef, useState } from "react";
import Link from 'next/link'
import { redirect } from 'next/navigation';
import { Image, CardText, Card, Stack, Button} from 'react-bootstrap'; 
import { Container, Col, Row } from 'react-bootstrap';
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
    const [isAlive, setAlive] = useState(false);
    const [isStream, setStream] = useState(false);
    const [isFrame, setFrame] = useState(black_image);
    const [isMPH, setMPH] = useState(0.0);
    const [steeringAngle, setSteeringAngle] = useState(0.0);

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

            setFrame(black_image);

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

            setFrame(black_image); 

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
        try {
            const req = await fetch(`http://${ipv4}:5002/isLive`);
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
            <Container fluid="lg"> 
                <Row className="justify-content-md-center"> 
                    <Col md="auto">
                        <div className="allStreamControls" onKeyDown={sendKeyPressToRos} > 
                            <div className="videoStream"> 
                                <div className="backgroundColor" /> 
                                <img className="videoFrame" src={isFrame} /> 
                            </div>
                        </div> 
                    </Col>
                    <Col md="auto" className="carAuto">
                        <div className="d-flex align-items-center gap-1 mb-3">
                            <Image className="mt-2 mx-1" src="../online-green.png" 
                                roundedCircle 
                                alt="me"
                                width="6px"
                                height="7px"/>
                            <CardText> Online </CardText>
                        </div>
                        <div className="gap-1">
                            <span> {isMPH} MPH</span>
                        </div>
                        <div className="gap-1">
                            <span> {steeringAngle}  ° Radians </span>
                        </div>
                    </Col>
                    <Col xs lg="2" className="justify-content-end p-2">
                        <Button variant="primary" className="rounded-circle p-0" size="sm" style={{
                            height: "6%", 
                            width: "15%", 
                            borderRadius: "50%"
                            }}>
                            <Image roundedCircle src="../power-switch.png" width="100%" height="100%"></Image>
                        </Button>
                    </Col>
                </Row> 
            </Container>
        </>
    );
}
