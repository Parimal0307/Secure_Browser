import React from 'react'
import './App.css'
import KeyEventMonitor from './KeyEventMonitor'

const App = () => {
  return (
    <div className='app'>
      <h1>Secure Browser</h1>
      <KeyEventMonitor />
    </div>
  )
}

export default App