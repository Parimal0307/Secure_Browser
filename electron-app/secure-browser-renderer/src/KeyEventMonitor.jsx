import { useEffect, useState } from "react";

function KeyEventMonitor() {
    const [events, setEvents] = useState([]);

    useEffect(() => {
        const handler = (e) => {
            console.log("Received:", e.detail);

            const newEvent = {
                message: e.detail.type,
                time: new Date(e.detail.timestamp).toLocaleTimeString()
            };

            setEvents((prev) => [newEvent, ...prev].slice(0, 10)); // keep last 10
        };

        window.addEventListener("native-key-event", handler);

        return () => {
            window.removeEventListener("native-key-event", handler);
        };
    }, []);

    return (
        <div>
            <h3>Blocked Events</h3>
            <ul>
                {events.map((e, index) => (
                    <li key={index}>
                        [{e.time}] {e.message}
                    </li>
                ))}
            </ul>
        </div>
    );
}

export default KeyEventMonitor;