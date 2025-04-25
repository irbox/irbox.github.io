import modules from './data/modules'
import Header from './components/Header'
import ModuleCard from './components/ModuleCard'

export default function App() {
  return (
    <div className="max-w-4xl mx-auto px-4 py-6">
      <Header />
      <div className="space-y-4 mt-6">
        {modules.map((mod, idx) => (
          <ModuleCard key={idx} module={mod} />
        ))}
      </div>
    </div>
  )
}
