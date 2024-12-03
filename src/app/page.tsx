"use client"
import Button from 'react-bootstrap/Button';
import Spinner from 'react-bootstrap/Spinner';
import { useEffect, useState } from "react";
import { redirect } from 'next/navigation';
import { useRouter } from 'next/navigation';
import { Check2Circle, XLg } from 'react-bootstrap-icons';
import "../../public/cool_car-removebg-preview.png";
import "./connectPage.css";


export default function Home() {
  const router = useRouter()

  const [isAlive, setAlive] = useState(false);
  const [isLoadingInital, setLoadingInital] = useState(true);
  const [isLoading, setLoading] = useState(false);

  useEffect(() => {
    initalChecks();
  }, []);

  const initalChecks = async() => {
    try {
      const req = await fetch("https://10.110.194.54:5000/heartbeat", {
        signal: AbortSignal.timeout(5000)
      }); 

      if (!req.ok){
        throw new Error(`Error: ${req.status}`);
      }

      const data = await req.json(); 
      if (data["is_alive"] !== false) {
        setAlive(true);
      }

    } catch(error) {
      setAlive(false);
    };
    setLoadingInital(false);
  }
  
  const checkCarHeartBeat = async () => {
    setLoading(true);
    try {
      const req = await fetch("https://10.110.194.54:5000/heartbeat", {
        signal: AbortSignal.timeout(5000)
      }); 

      if (!req.ok){
        throw new Error(`Error: ${req.status}`);
      }

      const data = await req.json(); 

      if (data["is_alive"] !== false) {
        setAlive(true);
        router.push('/control');
        return;
      }

      setAlive(false);
    } catch(error) {
      setLoading(false);
    };
    setLoading(false);
  }

  
  return (
  <>
    <div className="connectToScreen">
      <img src="./cool_car-removebg-preview.png" className='carImg'/> 
      <h1 className="cuteMessage">
        Welcome to Cal Poly Pomona's Car Controls
      </h1>
        <ul className="checksForCar">
          <li> 
            Alive?
            <span className="imgChecks">
              &nbsp;
              {isAlive === false && 
               isLoadingInital === true && 
               <Spinner animation='border' role='status' size='sm' aria-hidden="true" color="red" />}

              {isAlive === true 
                && isLoadingInital === false 
                && <Check2Circle color="green" size={20} />} 

              {isAlive === false 
                && isLoadingInital === false 
                && <XLg color="red" size={20}/>}
            </span>
          </li>
        </ul>
        <Button className="check-if-active" variant="primary" size="lg" onClick={checkCarHeartBeat}>
          {isLoading == true ? <Spinner
            animation="border" 
            role="status"
            size="sm"
            aria-hidden="true">
          </Spinner> :
            <div></div>
          }
          <span className={isLoading === true ? "visuallyHidden" : "visuallyVisible" }>
            {isLoading === true ? " Connecting..." : " Connect"}
          </span>
        </Button>
    </div> 
  </>
  );
}

